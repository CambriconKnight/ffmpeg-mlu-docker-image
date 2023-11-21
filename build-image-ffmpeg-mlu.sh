#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     build-image-ffmpeg-mlu.sh
# UpdateDate:   2022/06/10
# Description:  Build docker images for ffmpeg-mlu.
# Example:      ./build-image-ffmpeg-mlu.sh
#               ./build-image-ffmpeg-mlu.sh 16.04
#               ./build-image-ffmpeg-mlu.sh 18.04
# Depends:
#               Driver(neuware-mlu***-driver-dkms_*.*.*_all.deb)
#               CNToolkit(cntoolkit_*.*.*-1.ubuntu1*.04_amd64.deb)
#               CNCV(cncv_*.*.*-1.ubuntu1*.04_amd64.deb)
#               FFmpeg-MLU(https://github.com/Cambricon/ffmpeg-mlu)
#               FFmpeg(https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1)
# Notes:
# -------------------------------------------------------------------------------
#Dockerfile(16.04/18.04/CentOS)
OSVer="18.04"
if [[ $# -ne 0 ]];then OSVer="${1}";fi
# 0. Source env
source ./env.sh $OSVer
# CNToolkit_Version
CNToolkit_Version=`echo ${FILENAME_CNToolkit}|awk -F '-' '{print $1}'`
CNToolkit_Version_1=`echo ${CNToolkit_Version}|awk -F '_' '{print $1}'`
CNToolkit_Version_2=`echo ${CNToolkit_Version}|awk -F '_' '{print $2}'`
CNToolkit_Version="${CNToolkit_Version_1}-${CNToolkit_Version_2}"
#################### main ####################
# 1. check
if [ ! -d "$PATH_WORK" ];then
    mkdir -p $PATH_WORK
else
    echo "Directory($PATH_WORK): Exists!"
fi

# 2. Copy the dependent packages into the directory of $PATH_WORK
## Sync script
cp -rvf ./docker/build-ffmpeg-mlu.sh ./${PATH_WORK}
cp -rvf ./docker/install_cntoolkit.sh ./${PATH_WORK}
## Sync CNToolkit
pushd "${PATH_WORK}"
if [ -f "${FILENAME_CNToolkit}" ];then
    echo "File(${FILENAME_CNToolkit}): Exists!"
else
    echo -e "${red}File(${FILENAME_CNToolkit}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${FILENAME_CNToolkit} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${FILENAME_CNToolkit}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v ./dependent_files/${FILENAME_CNToolkit} ./${PATH_WORK}${none}"
    #Manual copy
    #cp -v /data/ftp/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi
popd
## Sync CNCV
pushd "${PATH_WORK}"
if [ -f "${FILENAME_CNCV}" ];then
    echo "File(${FILENAME_CNCV}): Exists!"
else
    echo -e "${red}File(${FILENAME_CNCV}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${FILENAME_CNCV} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${FILENAME_CNCV}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v ./dependent_files/${FILENAME_CNCV} ./${PATH_WORK}${none}"
    #Manual copy
    #cp -v /data/ftp/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNCV/cncv_0.4.602-1.ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi
popd

#1.build image
echo "====================== build image ======================"
sudo docker build -f ./docker/$FILENAME_DOCKERFILE \
    --build-arg cntoolkit_version=${CNToolkit_Version} \
    --build-arg cntoolkit_package=${FILENAME_CNToolkit} \
    --build-arg cncv_package=${FILENAME_CNCV} \
    --build-arg mlu_platform=${MLU_PLATFORM} \
    -t $NAME_IMAGE .
#2.save image
echo "====================== save image ======================"
sudo docker save -o $FILENAME_IMAGE $NAME_IMAGE
sudo chmod 664 $FILENAME_IMAGE
mv $FILENAME_IMAGE ./docker/
ls -lh ./docker/$FILENAME_IMAGE
#md5sum
#cd ./docker
#sudo md5sum ./$FILENAME_IMAGE > "./${FILENAME_IMAGE}.md5sum"
#cd -
