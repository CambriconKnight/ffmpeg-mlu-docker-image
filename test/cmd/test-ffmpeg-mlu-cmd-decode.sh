#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     test-ffmpeg-mlu-cmd-decode.sh
# UpdateDate:   2022/06/14
# Description:  Test ffmpeg-mlu based on command mode.
# Example:      ./test-ffmpeg-mlu-cmd-decode.sh
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
#################### Function ####################
# ffmpeg_mlu_cmd_decode
# $1: 视频文件/网址(多路并行测试时,尽量选用时间长一些的视频文件)
# $2: 设备ID(整数)
# $3: 启动路数(整数)
# $4: 视频格式(编码: h264_mluenc,hevc_mluenc; 解码: h264_mludec,hevc_mludec;)
ffmpeg_mlu_cmd_decode() {
    VIDEO=$1
    DEVICE_ID=$2
    CHANNEL_NUM=$3
    TYPE_CODE=$4
    LOG_PACH="log"
    if [ ! -d $LOG_PACH ];then
        mkdir -p $LOG_PACH
    fi
    i=1
    while((i <= $CHANNEL_NUM))
    do
        #ffmpeg -y -vsync 0 -c:v h264_mludec -device_id 0 -i ../data/jellyfish-3-mbps-hd-h264.mkv -f null -< /dev/null >> mludec_Process1.log 2>&1 &
        ffmpeg -y -vsync 0 -threads 1 -c:v ${TYPE_CODE} -device_id ${DEVICE_ID} -i ${VIDEO} -f null -< /dev/null >> ./${LOG_PACH}/mludec_Process${i}.log 2>&1 &
        let "i+=1"
    done
    # -y（全局参数） 覆盖输出文件而不询问。
    # -vsync 0
    # -c [：stream_specifier] codec（输入/输出，每个流） 选择一个编码器（当在输出文件之前使用）或×××（当在输入文件之前使用时）用于一个或多个流。codec 是×××/编码器的名称或 copy（仅输出）以指示该流不被重新编码。如：ffmpeg -i INPUT -map 0 -c:v libx264 -c:a copy OUTPUT
    # -c:v 与参数 -vcodec 一样，表示视频编码器。c 是 codec 的缩写，v 是video的缩写。
    # -device_id 选择使用的加速卡。支持设置的值的范围为：0 - INT_MAX。其中 INT_MAX 为加速卡总数减1。默认值为 0。
    # -i url（输入） 输入文件的网址
    # -f null
}

#############################################################
# 1. 设置环境变量
export WORK_DIR="/root/ffmpeg-mlu"
export NEUWARE_HOME=/usr/local/neuware
export LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64

# 2. 基于FFMPEG命令行方式验证MLU转码功能
# 2.1. 执行ffmpeg
cd /home/share/test/cmd
#解码1080P@30fps;
# for 365
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-h264.mkv 0 64 h264_mludec
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-hevc.mkv 0 66 hevc_mludec
# for 370
ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-h264.mkv 0 128 h264_mludec
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-hevc.mkv 0 132 hevc_mludec
echo -e "${green}"
# 2.2、查看转码后的log文件
#ls -lh *.log
#tail -n 10 ./log/mludec_Process*.log
echo -e "[Test ffmpeg-mlu ... Done] ${none}"

