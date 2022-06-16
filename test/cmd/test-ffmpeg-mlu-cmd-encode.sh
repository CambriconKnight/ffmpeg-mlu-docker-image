#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     test-ffmpeg-mlu-cmd-encode.sh
# UpdateDate:   2022/06/14
# Description:  基于 FFMPEG 命令行方式验证多路并行编码, 可用于上手阶段压测MLU板卡硬件编码能力.
# Example:      ./test-ffmpeg-mlu-cmd-encode.sh
# Depends:
#               Driver(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/Driver/neuware-mlu270-driver-dkms_4.9.8_all.deb)
#               CNToolkit(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb)
#               CNCV(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNCV/cncv_0.4.602-1.ubuntu16.04_amd64.deb)
#               FFmpeg-MLU(https://github.com/Cambricon/ffmpeg-mlu)
#               FFmpeg(https://gitee.com/mirrors/ffmpeg.git -b release/4.2 --depth=1)
# Notes:        多路并行测试时,尽量选用时间长一些的视频文件, 以避免多路启动先后顺序造成压测力度不够.
# -------------------------------------------------------------------------------
#Font color
none="\033[0m"
green="\033[0;32m"
red="\033[0;31m"
yellow="\033[1;33m"
white="\033[1;37m"
#################### Function ####################
# ffmpeg_mlu_cmd_encode
# $1: 视频文件/网址(多路并行测试时,尽量选用时间长一些的视频文件)
# $2: 设备ID(整数)
# $3: 启动路数(整数)
# $4: 视频格式(编码: h264_mluenc,hevc_mluenc; 解码: h264_mludec,hevc_mludec;)
ffmpeg_mlu_cmd_encode() {
    VIDEO=$1
    DEVICE_ID=$2
    CHANNEL_NUM=$3
    TYPE_CODE=$4
    LOG_PACH="log"
    if [ ! -d $LOG_PACH ];then
        mkdir -p $LOG_PACH
    fi
    if [ ! -f "${VIDEO}" ];then
        echo -e "${red}File(${VIDEO}): Not exist!${none}"
        echo -e "${yellow}  Please download ${VIDEO} from baidudisk(cat ../../dependent_files/README.md)!${none}"
        echo -e "${yellow}  For further information, please contact us.${none}"
        exit -1
    fi
    #TEST
    i=1
    while((i <= $CHANNEL_NUM))
    do
        #ffmpeg -y -r 30 -i ../data/csgo_1920x1080_420_8_30_3602.y4m -c:v h264_mluenc -device_id 0 -rc vbr -b:v 10M -profile:v high -bf 3 -vsync 0 -f H264 -< /dev/null >> mluenc_Process1.log 2>&1 &
        ffmpeg -y -r 30 -i ${VIDEO} -threads 1 -c:v ${TYPE_CODE} -device_id ${DEVICE_ID} -rc vbr -b:v 10M -profile:v high -bf 3 -vsync 0 -f H264 -< /dev/null >> ./${LOG_PACH}/mluenc_Process${i}.log 2>&1 &
        let "i+=1"
    done
    # -y（全局参数） 覆盖输出文件而不询问。
    # -r [：stream_specifier] fps（输入/输出，每个流） 设置帧率（Hz值，分数或缩写）。作为输入选项，忽略存储在文件中的任何时间戳，根据速率生成新的时间戳。这与用于-framerate选项不同（它在FFmpeg的旧版本中使用的是相同的）。如果有疑问，请使用-framerate而不是输入选项-r。作为输出选项，复制或丢弃输入帧以实现恒定输出帧频fps。
    # -i url（输入） 输入文件的网址
    # -c [：stream_specifier] codec（输入/输出，每个流） 选择一个编码器（当在输出文件之前使用）或×××（当在输入文件之前使用时）用于一个或多个流。codec 是×××/编码器的名称或 copy（仅输出）以指示该流不被重新编码。如：ffmpeg -i INPUT -map 0 -c:v libx264 -c:a copy OUTPUT
    # -c:v 与参数 -vcodec 一样，表示视频编码器。c 是 codec 的缩写，v 是video的缩写。
    # -device_id 选择使用的加速卡。支持设置的值的范围为：0 - INT_MAX。其中 INT_MAX 为加速卡总数减1。默认值为 0。
    # -rc v`br 编码码率控制模式参数。
    # -b:v 改变文件的视频码率
    # -profile:v 设置编码档次。支持设置的值为：baseline、main、high和 high444p。默认值为 high。
    # -bf 3 -bf frames 使用frames个B 帧，支持mpeg1,mpeg2,mpeg4（即如果-bf 2的话，在两个非b帧中间隔的b帧数目为2，即IBBPBBPBBP结构）
    # -vsync 0
    # -f H264
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

# 编码1080p@30fps:
# for 365
#ffmpeg_mlu_cmd_encode ../data/csgo_1920x1080_420_8_30_3602.y4m 0 12 h264_mluenc
# for 370
ffmpeg_mlu_cmd_encode ../data/csgo_1920x1080_420_8_30_3602.y4m 0 24 h264_mluenc
echo -e "${green}"
# 2.2、查看转码后的log文件
#ls -lh *.log
#tail -n 10 ./log/mluenc_Process*.log
echo -e "[Monitor Video Encoder 10-11 On Host:]"
echo -e "[cd ./tools && ./test-cnmon.sh 0] ${none}"

