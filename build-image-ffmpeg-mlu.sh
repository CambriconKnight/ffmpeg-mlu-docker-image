#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     build-image-ffmpeg-mlu.sh
# UpdateDate:   2022/02/08
# Description:  Build docker images for ffmpeg-mlu.
# Example:      ./build-image-ffmpeg-mlu.sh
# Depends:
#               Driver(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/Driver/neuware-mlu270-driver-dkms_4.9.8_all.deb)
#               CNToolkit(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb)
#               CNCV(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNCV/cncv_0.4.602-1.ubuntu16.04_amd64.deb)
#               FFmpeg-MLU(https://github.com/Cambricon/ffmpeg-mlu)
#               FFmpeg(https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1)
# Notes:
# -------------------------------------------------------------------------------
#Dockerfile(16.04/18.04/CentOS)
OSVer="16.04"
if [[ $# -ne 0 ]];then OSVer="${1}";fi
# 0. Source env
source ./env.sh $OSVer
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
## Sync CNToolkit
pushd "${PATH_WORK}"
if [ -f "${FILENAME_MLU270_CNToolkit}" ];then
    echo "File(${FILENAME_MLU270_CNToolkit}): Exists!"
else
    echo -e "${red}File(${FILENAME_MLU270_CNToolkit}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${FILENAME_MLU270_CNToolkit} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${FILENAME_MLU270_CNToolkit}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v ./dependent_files/${FILENAME_MLU270_CNToolkit} ./${PATH_WORK}${none}"
    #Manual copy
    #cp -v /data/ftp/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi
popd
## Sync CNCV
pushd "${PATH_WORK}"
if [ -f "${FILENAME_MLU270_CNCV}" ];then
    echo "File(${FILENAME_MLU270_CNCV}): Exists!"
else
    echo -e "${red}File(${FILENAME_MLU270_CNCV}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${FILENAME_MLU270_CNCV} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${FILENAME_MLU270_CNCV}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v ./dependent_files/${FILENAME_MLU270_CNCV} ./${PATH_WORK}${none}"
    #Manual copy
    #cp -v /data/ftp/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNCV/cncv_0.4.602-1.ubuntu16.04_amd64.deb ./ffmpeg-mlu
    exit -1
fi
popd

#1.build image
echo "====================== build image ======================"
sudo docker build -f ./docker/$FILENAME_DOCKERFILE \
    --build-arg cntoolkit_package=${FILENAME_MLU270_CNToolkit} \
    --build-arg cncv_package=${FILENAME_MLU270_CNCV} \
    -t $NAME_IMAGE .
#2.save image
echo "====================== save image ======================"
sudo docker save -o $FILENAME_IMAGE $NAME_IMAGE
sudo chmod 664 $FILENAME_IMAGE
mv $FILENAME_IMAGE ./docker/
ls -lh ./docker/$FILENAME_IMAGE
