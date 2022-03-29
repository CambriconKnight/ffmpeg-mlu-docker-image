#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     test.sh
# UpdateDate:   2022/03/21
# Description:  Test ffmpeg-mlu based on API mode.
# Example:      ./test-ffmpeg-mlu-api.sh
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
#############################################################
# 1. 设置环境变量
export WORK_DIR="/root/ffmpeg-mlu"
export NEUWARE_HOME=/usr/local/neuware
export LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64

# 2. 基于FFMPEG API方式验证MLU转码功能
# 2.1. 执行
#Usage: ./transcoder_vid2vid <file_path> <dst_w> <dst_h> <device_id> <thread_num> <save_flag>
./transcoder_vid2vid ../../data/test.mp4 352 288 0 1 1
#./transcoder_vid2vid ../../data/A002.mp4 352 288 0 50 0
#./transcoder_vid2vid rtsp://username1:password1@10.100.9.70:8554/cars-6.mkv 352 288 0 1 0
echo -e "${green}"
# 2.2. 查看转码后的视频文件
ls -lh ./_thread_*.h264
echo -e "[Test ffmpeg-mlu ... Done] ${none}"