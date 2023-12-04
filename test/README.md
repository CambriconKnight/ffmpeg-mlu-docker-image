# 基于ffmpeg-mlu的性能测试

```bash
.
├── api                                 #API方式测试
│   ├── clean.sh
│   ├── test-ffmpeg-mlu-api.sh
│   └── transcode
├── cmd                                 #命令方式测试
│   ├── test-ffmpeg-mlu-cmd-decode.sh   #命令方式测试370解码性能
│   ├── test-ffmpeg-mlu-cmd-encode.sh   #命令方式测试370编码性能
│   └── test-ffmpeg-mlu-cmd.sh
├── data                                #测试数据
│   └── test.mp4
└── README.md                           #README
```

# 基于cncodec的性能测试
```bash
if [ -e /usr/local/bin/jpeg_dec_perf ]; then exit 0; fi
# 基于 cncodec 的性能测试
#编译测试代码
cd /usr/local/neuware/samples/cncodec_v3
if [ ! -d build ]; then mkdir -p build; fi
cd build && cmake ../ && make
#拷贝可执行程序到系统用户bin目录
cp -rvf /usr/local/neuware/samples/cncodec_v3/bin/* /usr/local/bin
#测试图片解码(jpeg_dec_perf使用方法可查看帮助信息)
jpeg_dec_perf -i /home/share/test/data/images/a.jpg -thread 10 -mlu 0 -l 1000 -monitor
#参考以下命令查看硬件资源占用情况
#while true;do echo "==========================================";cnmon info -c 0 | grep -E "Image Codec|Board|Device CPU Chip|DDR|Chip|Memory|Used|Usage|Power";sleep 0.5;done

#测试视频解码
#解码序
video_dec_perf -i /home/share/test/data/jellyfish-3-mbps-hd-h264.mkv -thread 10 -fps 30 -mlu 0 -monitor -decode_order
#显示序
#video_dec_perf -i /home/share/test/data/jellyfish-3-mbps-hd-h264.mkv -thread 10 -fps 30 -mlu 0 -monitor
```