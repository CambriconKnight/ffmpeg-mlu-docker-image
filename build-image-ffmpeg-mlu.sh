#/bin/bash
set -e

# 1. copy your neuware package into the directory of ffmpeg-mlu-rel
# 2. docker build -f dockerfile/Dockerfile --build-arg mlu_platform=MLU270 --build-arg neuware_package=neuware-mlu270-1.4.0-1_Ubuntu16.04_amd64.deb -t ubuntu_ffmpeg-mlu:v1 .
# 3. docker run -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --privileged -v /dev:/dev --net=host --pid=host --ipc=host -v $HOME/.Xauthority -it --name ubuntu_ffmpeg-mlu  -v $PWD:/workspace ubuntu_ffmpeg-mlu:v1
# 4. if docker is ready, can execute it by docker start ubuntu_ffmpeg-mlu / docker exec -it  ubuntu_ffmpeg-mlu /bin/bash

#Version
VERSION="v1.5.0"
PATH_FFMPEG_MLU="ffmpeg-mlu"
neuware_version=neuware-mlu270-1.5.0
neuware_package_name="${neuware_version}-1_Ubuntu16.04_amd64.deb"
NAME_IMAGE="ubuntu16.04_ffmpeg-mlu:$VERSION"
FILENAME_IMAGE="ubuntu16.04_ffmpeg-mlu-$VERSION.tar.gz"

none="\033[0m"
green="\033[0;32m"
red="\033[0;31m"
yellow="\033[1;33m"

##0.git clone
if [ ! -d "${PATH_FFMPEG_MLU}" ];then
    #git clone git@github.com:Cambricon/ffmpeg-mlu.git
    #git clone git@github.com:CambriconKnight/ffmpeg-mlu.git
    git clone https://git@github.com/CambriconKnight/ffmpeg-mlu.git
    #cd ffmpeg-mlu
    #git clone https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1
    #cd ../
    #cd ffmpeg
    #git apply ../ffmpeg4.2_mlu.patch
    #cd ../../
else
    echo "Directory(${PATH_FFMPEG_MLU}): Exists!"
fi
cd "${PATH_FFMPEG_MLU}"
# del .git
find . -name ".git" | xargs rm -Rf

##copy your neuware package into the directory of ffmpeg-mlu
if [ -f "${neuware_package_name}" ];then
    echo "File(${neuware_package_name}): Exists!"
else
    echo -e "${red}File(${neuware_package_name}): Not exist!${none}"
    echo -e "${yellow}Copy your neuware package(${neuware_package_name}) into the directory of ffmpeg-mlu!${none}"
    echo -e "${yellow}eg:cp -v /data/ftp/v1.5.0/neuware/${neuware_package_name} ./${PATH_FFMPEG_MLU}${none}"
    #Manual copy
    #cp -v /data/ftp/v1.5.0/neuware/neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi

#1.build image
echo "====================== build image ======================"
docker build -f ./dockerfile/Dockerfile --build-arg neuware_version=${neuware_version} -t $NAME_IMAGE .
#2.save image
echo "====================== save image ======================"
docker save -o $FILENAME_IMAGE $NAME_IMAGE
mv $FILENAME_IMAGE ../
cd ../
ls -la $FILENAME_IMAGE
