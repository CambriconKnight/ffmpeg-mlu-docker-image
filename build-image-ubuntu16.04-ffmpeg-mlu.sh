#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     build-image-ubuntu16.04-ffmpeg-mlu.sh
# UpdateDate:   2021/02/23
# Description:  Build docker images for ffmpeg-mlu.
# Example:      ./build-image-ubuntu16.04-ffmpeg-mlu.sh
# Depends:
#               driver(ftp://download.cambricon.com:8821/product/MLU270/1.6.602/driver/neuware-mlu270-driver-dkms_4.9.2_all.deb)
#               cntoolkit(ftp://download.cambricon.com:8821/product/MLU270/1.6.602/cntoolkit/X86_64/ubuntu16.04/cntoolkit_1.7.2-1.ubuntu16.04_amd64.deb)
#               ffmpeg-mlu(https://github.com/Cambricon/ffmpeg-mlu.git)
#               ffmpeg(https://gitee.com/mirrors/ffmpeg.git)
# Notes:
# -------------------------------------------------------------------------------
#################### function ####################
help_info() {
    echo "
Build docker images for ffmpeg-mlu.
Usage:
    $0 <command> [arguments]
The commands are:
    -h      Help info.
Examples:
    $0 -h
    $0
Use '$0 -h' for more information about a command.
    "
}
#################### main ####################
# Source env
source "./env.sh"

#neuware or cntoolkit
neuware_version=cntoolkit-1.7.2
neuware_package_name="cntoolkit_1.7.2-1.ubuntu16.04_amd64.deb"

##0.git clone
if [ ! -d "${PATH_WORK}" ];then
    git clone https://github.com/Cambricon/ffmpeg-mlu.git
else
    echo "Directory(${PATH_WORK}): Exists!"
fi
cd "${PATH_WORK}"
# del .git
find . -name ".git" | xargs rm -Rf

## copy the dependent packages into the directory of $PATH_WORK
if [ -f "${neuware_package_name}" ];then
    echo "File(${neuware_package_name}): Exists!"
else
    echo -e "${red}File(${neuware_package_name}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${neuware_package_name} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${neuware_package_name}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v /data/ftp/mlu270/$VER/cntoolkit/X86_64/ubuntu16.04/${neuware_package_name} ./${PATH_WORK}${none}"
    #Manual copy
    #cp -v /data/ftp/mlu270/1.6.602/cntoolkit/X86_64/ubuntu16.04/cntoolkit_1.4.110-1.ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi

#1.build image
echo "====================== build image ======================"
sudo docker build -f ../$FILENAME_DOCKERFILE \
    --build-arg neuware_version=${neuware_version} \
    --build-arg neuware_package=${neuware_package_name} \
    -t $NAME_IMAGE .

#2.save image
echo "====================== save image ======================"
sudo docker save -o $FILENAME_IMAGE $NAME_IMAGE
mv $FILENAME_IMAGE ../
cd ../
ls -la $FILENAME_IMAGE