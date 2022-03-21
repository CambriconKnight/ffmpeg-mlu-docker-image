#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     build-ffmpeg-mlu.sh
# UpdateDate:   2022/02/08
# Description:  Build ffmpeg-mlu.
# Example:      ./build-ffmpeg-mlu.sh
# Depends:
#               Driver(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/Driver/neuware-mlu270-driver-dkms_4.9.8_all.deb)
#               CNToolkit(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb)
#               CNCV(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNCV/cncv_0.4.602-1.ubuntu16.04_amd64.deb)
#               FFmpeg-MLU(https://github.com/Cambricon/ffmpeg-mlu)
#               FFmpeg(https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1)
# Notes:
# -------------------------------------------------------------------------------
#Font color
none="\033[0m"
green="\033[0;32m"
red="\033[0;31m"
yellow="\033[1;33m"
white="\033[1;37m"
#ENV
PATH_WORK="ffmpeg-mlu"
WORK_DIR="/root/ffmpeg-mlu"
#############################################################
# 1. Compile and install FFmpeg-MLU
## 1.1. Download FFmpeg-MLU
cd $WORK_DIR
PATH_WORK_TMP="ffmpeg-mlu"
if [ -f "${FILENAME_FFMPEG_MLU}" ];then
    echo -e "${green}File(${FILENAME_FFMPEG_MLU}): Exists!${none}"
    # $FILENAME_FFMPEG_MLU 压缩包中已经包含了ffmpeg-mlu补丁 + ffmpeg4.2
    tar zxvf $FILENAME_FFMPEG_MLU
else
    echo -e "${red}File(${FILENAME_FFMPEG_MLU}): Not exist!${none}"
    echo -e "${yellow}1.Please download ${FILENAME_FFMPEG_MLU} from FTP(ftp://download.cambricon.com:8821/***)!${none}"
    echo -e "${yellow}  For further information, please contact us.${none}"
    echo -e "${yellow}2.Copy the dependent packages(${FILENAME_FFMPEG_MLU}) into the directory!${none}"
    echo -e "${yellow}  eg:cp -v ./dependent_files/${FILENAME_FFMPEG_MLU} ./${PATH_WORK}${none}"
    echo -e "${green}3.Downloading automatically......${none}"

    if [ ! -d "${PATH_WORK_TMP}" ];then
        git clone https://github.com/Cambricon/ffmpeg-mlu
    else
        echo "Directory($PATH_WORK_TMP): Exists!"
    fi

    ## 1.2. Download FFmpeg
    pushd $PATH_WORK_TMP
    git clone https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1
    popd

    ## 1.3 Patch
    pushd $PATH_WORK_TMP/ffmpeg
    git apply ../ffmpeg4.2_mlu.patch
    popd
fi

## 1.4. 设置环境变量
export NEUWARE_HOME=/usr/local/neuware
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64

## 1.5. 编译FFmpeg-MLU
pushd $PATH_WORK_TMP/ffmpeg
./configure  --enable-gpl \
                --extra-cflags="-I${NEUWARE_HOME}/include" \
                --extra-ldflags="-L${NEUWARE_HOME}/lib64" \
                --extra-libs="-lcncodec -lcnrt -ldl -lcndrv" \
                --enable-ffplay \
                --enable-ffmpeg \
                --enable-mlumpp \
                --enable-gpl \
                --enable-version3 \
                --enable-nonfree \
                --disable-static \
                --enable-shared \
                --disable-debug \
                --enable-stripping \
                --enable-optimizations \
                --enable-avresample
make -j4 && make install
popd

## 1.6. 编译FFmpeg-mlu依赖库
### a.建议系统中没有原生FFmpeg,或已卸载其他版本的FFmpeg
### b.建议将FFmpeg—MLU直接安装到系统目录下
### 1.6.1. 安装完成cncv库(拷贝cncv安装包到当前目录下)
#dpkg -i cncv_0.4.602-1.ubuntu18.04_amd64.deb
### 1.6.2. 编译mlu_op算子
mkdir -pv $PATH_WORK_TMP/mlu_op/build
pushd $PATH_WORK_TMP/mlu_op/build
cmake .. && make -j4
popd
### 1.6.3. 拷贝libeasyOP.so 到NEUWARE_HOME路径下
cp -rvf $PATH_WORK_TMP/mlu_op/lib/libeasyOP.so ${NEUWARE_HOME}/lib64
ls -lh ${NEUWARE_HOME}/lib64/libeasyOP.so
echo -e "${green}[Build ${PATH_WORK_TMP}... Done] ${none}"
cd $WORK_DIR
#############################################################
# 2.Test FFMpeg-MLU
#./test-ffmpeg-mlu.sh