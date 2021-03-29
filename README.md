# ffmpeg-mlu Docker Images #

Build docker images for [ffmpeg-mlu](https://github.com/Cambricon/ffmpeg-mlu).

## Directory tree ##

```bash
.
├── build-image-ubuntu16.04-ffmpeg-mlu.sh
├── README.md
├── load-image-ubuntu16.04-ffmpeg-mlu.sh
└── run-container-ubuntu16.04-ffmpeg-mlu.sh
```

## Clone ##
```bash
git clone https://github.com/CambriconKnight/ffmpeg-mlu-docker-image.git
```
```bash
cam@cam-3630:/data/docker$ git clone https://github.com/CambriconKnight/ffmpeg-mlu-docker-image.git
Cloning into 'ffmpeg-mlu-docker-image'...
remote: Enumerating objects: 24, done.
remote: Counting objects: 100% (24/24), done.
remote: Compressing objects: 100% (18/18), done.
remote: Total 24 (delta 10), reused 16 (delta 5), pack-reused 0
Unpacking objects: 100% (24/24), done.
Checking connectivity... done.
cam@cam-3630:/data/docker$ ls
build-image-ubuntu16.04-ffmpeg-mlu.sh  load-image-ubuntu16.04-ffmpeg-mlu.sh  README.md  run-container-ubuntu16.04-ffmpeg-mlu.sh
cam@cam-3630:/data/docker$
```

## Build ##
```bash
./build-image-ubuntu16.04-ffmpeg-mlu.sh
```

## Load ##
```bash
./load-image-ubuntu16.04-ffmpeg-mlu.sh
```

## Run ##
```bash
./run-container-ubuntu16.04-ffmpeg-mlu.sh
```

## Update ##
Execute the following command when logging in to the container for the first time.
```bash
#1、更新软件列表、更新软件
apt-get update && apt-get upgrade -y
#2、安装cmake
apt-get install cmake -y
#3、安装cnml 和 cnplugin
cd /var/neuware-mlu270-1.5.0
dpkg -i cnml_7.7.0-1.ubuntu16.04_amd64.deb cnplugin_1.8.0-1.ubuntu16.04_amd64.deb
cd -
#4、编译mlu_op
cd ~/ffmpeg-mlu/mlu_op
mkdir build
cd build
cmake ../ && make -j
#5、安装mlu_op
#make install
ls -la ../lib/libeasyOP.so
cp ../lib/libeasyOP.so /usr/local/neuware/lib64/
ls -la /usr/local/neuware/lib64/libeasyOP.so
#6、设置环境变量
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/neuware/lib64
echo $LD_LIBRARY_PATH
```

## Test ##
### Command ###
```bash
#转码验证
#基于FFMPEG转码有两种方式
#1、命令行方式
#1.1、执行ffmpeg
cd ~/ffmpeg-mlu/ffmpeg/build/
./ffmpeg -y -c:v h264_mludec -resize 352x288 -i /home/cam/data/jellyfish-3-mbps-hd-h264_1800.mkv -c:v h264_mluenc output_cif.h264
#1.2、查看转码后的视频文件
cd ~/ffmpeg-mlu/ffmpeg/build/
ls -lh ./output_cif.h264
cp output_cif.h264 /home/cam/
ls -lh /home/cam/output_cif.h264
```

### API ###
```bash
#转码验证
#基于FFMPEG转码有两种方式
#2、API接口调用方式
#参考DEMO（FTP单独提供）：ffmpeg-mlu_vid2vid_transcoder
#2.1、拷贝代码到~/ffmpeg-mlu/
cd ~/ffmpeg-mlu
cp /home/cam/ffmpeg-mlu_apps_vid2vid.tar.gz ./
tar zxvf ffmpeg-mlu_apps_vid2vid.tar.gz
ls -la ~/ffmpeg-mlu/ffmpeg-mlu_apps/
#2.2、编译ffmpeg动态库
cd ~/ffmpeg-mlu/ffmpeg
mkdir install
./configure --prefix="./install/" --enable-shared --enable-gpl --enable-version3 --enable-mlumpp --extra-cflags="-I/usr/local/neuware/include" --extra-ldflags="-L/usr/local/neuware/lib64" --extra-libs="-lcncodec -lcnrt -ldl -lcndrv"  --enable-debug --disable-asm --disable-stripping --disable-optimizations
make -j && make install
ls -la /root/ffmpeg-mlu/ffmpeg/install
#2.3、部署ffmpeg动态库到apps第三方依赖目录
cd /root/ffmpeg-mlu/ffmpeg/install
mkdir /root/ffmpeg-mlu/ffmpeg-mlu_apps/ffmpeg-mlu_vid2vid_transcoder/3rdparty/ffmpeg/
cp -r /root/ffmpeg-mlu/ffmpeg/install/* /root/ffmpeg-mlu/ffmpeg-mlu_apps/ffmpeg-mlu_vid2vid_transcoder/3rdparty/ffmpeg/
ls -la /root/ffmpeg-mlu/ffmpeg-mlu_apps/ffmpeg-mlu_vid2vid_transcoder/3rdparty/ffmpeg/
#2.4、增加ffmpeg/lib到环境变量
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/ffmpeg-mlu/ffmpeg-mlu_apps/ffmpeg-mlu_vid2vid_transcoder/3rdparty/ffmpeg/lib
echo $LD_LIBRARY_PATH
#2.5、编译ffmpeg-mlu_vid2vid_transcoder
cd ~/ffmpeg-mlu/ffmpeg-mlu_apps/ffmpeg-mlu_vid2vid_transcoder/
mkdir build
cd build
cmake .. && make -j
ls -la
#2.6、运行app，验证功能
#Usage: ./vid2vid_trans <file_path> <dst_w> <dst_h> <device_id> <thread_num> <save_flag>
./vid2vid_trans /home/cam/data/jellyfish-3-mbps-hd-h264_1800.mkv 352 288 0 10 1
#2.7、查看转码后的视频文件
ls -la _thread_*
cp _thread_* /home/cam/data
ls -lh /home/cam/data/_thread_*
```