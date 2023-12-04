#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     test-ffmpeg-mlu-cmd-decode-plus.sh
# UpdateDate:   2022/06/14
# Description:  基于 FFMPEG 命令行方式验证多路并行解码, 可用于上手阶段压测MLU板卡硬件解码能力.
#               此plus脚本在运行核心业务过程中可以实时显示显示业务日志文件并记录到日志文件，并且还会打印cnmon相关信息到日志文件。
# Example:      ./test-ffmpeg-mlu-cmd-decode-plus.sh [VideoFile] [DeviceID] [ChannelNum] [TypeCode]
#               ./test-ffmpeg-mlu-cmd-decode-plus.sh ../data/jellyfish-3-mbps-hd-h264.mkv 0 128 h264_mludec
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
        #ffmpeg -y -vsync 0 -c:v h264_mludec -device_id 0 -i ../data/jellyfish-3-mbps-hd-h264.mkv -f null -< /dev/null >> mludec_Process1.log 2>&1 &
        #ffmpeg -y -nostats -r 30 -vsync 0 -threads 1 -c:v ${TYPE_CODE} -hwaccel mlu -hwaccel_output_format mlu -hwaccel_device ${DEVICE_ID} -device_id ${DEVICE_ID} -i ${VIDEO} -f null -< /dev/null > ./${LOG_PACH}/mludec_Process${i}.log 2>&1 &
        ffmpeg -y -r 30 -vsync 0 -threads 1 -c:v ${TYPE_CODE} -hwaccel mlu -hwaccel_output_format mlu -hwaccel_device ${DEVICE_ID} -device_id ${DEVICE_ID} -i ${VIDEO} -f null -< /dev/null > ./${LOG_PACH}/mludec_Process${i}.log 2>&1 &
        let "i+=1"
        #sleep 0.001 #不能加sleep，否则容易压不倒最大路数。
        PID_LastProcess=$!
    done
    # -y（全局参数） 覆盖输出文件而不询问。
    # -vsync 0
    # -c [：stream_specifier] codec（输入/输出，每个流） 选择一个编码器（当在输出文件之前使用）或×××（当在输入文件之前使用时）用于一个或多个流。codec 是×××/编码器的名称或 copy（仅输出）以指示该流不被重新编码。如：ffmpeg -i INPUT -map 0 -c:v libx264 -c:a copy OUTPUT
    # -c:v 与参数 -vcodec 一样，表示视频编码器。c 是 codec 的缩写，v 是video的缩写。
    # -hwaccel 使用hwaccel 硬件加速模式，解码h264 码流，不涉及host 侧和device 侧之间的数据拷贝，全程在设备侧进行。关联参数有hwaccel_output_format&hwaccel_device
    # -device_id 选择使用的加速卡。支持设置的值的范围为：0 - INT_MAX。其中 INT_MAX 为加速卡总数减1。默认值为 0。
    # -i url（输入） 输入文件的网址
    # -f null
    # -loglevel info
    # -nostats ：不输出视频相关信息
    # -r 设定帧速率
}
watch_log_info() {
    #实时显示日志文件
    echo -e "All log files: ${1}"
    echo "PID_LastProcess: $PID_LastProcess"
    #sleep 0.2
    tail -f ${1} --pid=$PID_LastProcess
    sleep 0.1
    #业务结束后删除【实时记录cnmon关键信息】的进程
    #ps -aux | grep test-ffmpeg-mlu-cmd-decode
    kill -9 $PID_CNMONProcess
    sleep 0.1
    #######################################################
    #显示所有日志文件
    echo -e "${green}#####################################"
    echo -e "All log files: ${1}"
    ls ${1}
    #统计日志文件的个数
    Number_Log_Files=`ls -l ${1}|grep "^-"|wc -l`
    #echo -e "平均帧率: $Number_Log_Files"
    echo -e "视频处理路数: $Number_Log_Files"
    #统计相关进程信息，避免造成不受控制的僵尸进程
    echo -e "PID_LastProcess: $PID_LastProcess"
    echo -e "PID_CNMONProcess: $PID_CNMONProcess"
    echo -e "[Monitor Video Decoder 0-3、5-9 On Host:]"
    echo -e "[cd ../../tools && ./test-cnmon.sh 0]"
    echo -e "#####################################${none}"
}
#############################################################
# 1. 设置环境变量
export WORK_DIR="/root/ffmpeg-mlu"
export NEUWARE_HOME=/usr/local/neuware
export LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64
PID_LastProcess=10000000000
PID_CNMONProcess=10000000000

# 检查参数
# $1: VideoFile 视频文件/网址(多路并行测试时,尽量选用时间长一些的视频文件)
# $2: DeviceID 设备ID(整数)
# $3: ChannelNum 启动路数(整数)
# $4: TypeCode 视频格式(编码: h264_mluenc,hevc_mluenc; 解码: h264_mludec,hevc_mludec;)

if [[ $# -ne 4 ]];then
    echo -e "\033[1;33m Usage: $0 [VideoFile] [DeviceID] [ChannelNum] [TypeCode] \033[0m"
    exit -1
fi
VideoFile=$1
DeviceID=$2
ChannelNum=$3
TypeCode=$4
# 2. 基于FFMPEG命令行方式验证MLU转码功能
# 2.1. 执行ffmpeg
cd /home/share/test/cmd
#解码1080P@30fps;
# for 365
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-h264.mkv 0 64 h264_mludec
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-hevc.mkv 0 66 hevc_mludec
# for 370
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-h264.mkv 0 128 h264_mludec
#ffmpeg_mlu_cmd_decode ../data/jellyfish-3-mbps-hd-hevc.mkv 0 132 hevc_mludec
#解码1080P@30fps;
ffmpeg_mlu_cmd_decode ${VideoFile} ${DeviceID} ${ChannelNum} ${TypeCode}
#实时记录cnmon关键信息
StringGrep="Video|Board|Device CPU Chip|DDR|Chip|Memory|Used|Usage|Power"
LOG_FILENAME_CNMON="./log/mludec_cnmon.log"
cnmon -c ${DeviceID} > ${LOG_FILENAME_CNMON}; sleep 0.1;
#while true; do cnmon info -c ${DeviceID} | grep -E "${StringGrep}" >> ${LOG_FILENAME_CNMON};sleep 0.1;cnmon -c ${DeviceID} >> ${LOG_FILENAME_CNMON};sleep 0.1;done &
while true; do cnmon info -c ${DeviceID} >> ${LOG_FILENAME_CNMON};sleep 0.1;cnmon -c ${DeviceID} >> ${LOG_FILENAME_CNMON};sleep 0.1;done &
PID_CNMONProcess=$!
echo "PID_CNMONProcess: $PID_CNMONProcess"
#sleep 0.2
# 2.2. 查看ffmpeg执行业务后的log文件
LOG_FILENAME_PROCESS="./log/mludec_Process*.log"
echo "${LOG_FILENAME_PROCESS}"
watch_log_info "${LOG_FILENAME_PROCESS}"



