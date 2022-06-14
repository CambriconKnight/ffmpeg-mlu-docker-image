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
FILENAME_FFMPEG_MLU="ffmpeg-mlu-master.tar.gz"
MLU_PLATFORM="$1"
PARAM_NUM=$#
function helper() {
   echo ""
   echo "***** ffmpeg compile script instructions *****"
   echo " $0 <mlu_platform> <neuware_home>"
   echo "    <mlu_platform>: Required. Choose from MLU200 or MLU370, depending on your situation"
   echo ""
   echo "    e.g. if your hardware platform is MLU200, use this:"
   echo "      $0 MLU200"
   echo "    e.g. if your hardware platform is MLU370, use this:"
   echo "      $0 MLU370"
   echo "**********************************************"
   echo ""
}

if [ ${PARAM_NUM} -ne 1 ] && [ ${PARAM_NUM} -ne 2 ];then
   helper
   exit 0
fi
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
fi

## 1.4. 设置环境变量
export NEUWARE_HOME=/usr/local/neuware
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64

## 1.5. 编译FFmpeg-MLU
pushd $PATH_WORK_TMP
#删除--prefix选项是配置安装的路径,将FFmpeg—MLU直接安装到系统目录下
sed -i '/--prefix=/d' ./compile_ffmpeg.sh
#使用FFmpeg-MLU自带的编译脚本进行编译
./compile_ffmpeg.sh ${MLU_PLATFORM}
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