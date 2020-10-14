# ffmpeg-mlu Docker Images #

Build docker images for [ffmpeg-mlu](https://github.com/Cambricon/ffmpeg-mlu).

## Directory tree ##

```bash
.
├── build-image-ffmpeg-mlu.sh
├── README.md
├── load-image-ffmpeg.sh
└── run-container-ffmpeg.sh
```

## Build ##

```bash
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ls -la
total 28
drwxrwxr-x 2 cam cam 4096 10月 14 17:31 .
drwxrwxr-x 7 cam cam 4096 10月 13 16:32 ..
-rwxrwxr-x 1 cam cam 2341 10月 14 16:44 build-image-ffmpeg-mlu.sh
-rwxrwxr-- 1 cam cam  690 10月 14 16:01 load-image-ffmpeg.sh
-rw-rw-r-- 1 cam cam 7746 10月 14 17:30 README.md
-rwxrwxr-x 1 cam cam 1599 10月 14 15:59 run-container-ffmpeg.sh
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ./build-image-ffmpeg-mlu.sh
Cloning into 'ffmpeg-mlu'...
remote: Enumerating objects: 113, done.
remote: Counting objects: 100% (113/113), done.
remote: Compressing objects: 100% (70/70), done.
remote: Total 113 (delta 52), reused 94 (delta 34), pack-reused 0
Receiving objects: 100% (113/113), 1.38 MiB | 437.00 KiB/s, done.
Resolving deltas: 100% (52/52), done.
Checking connectivity... done.
File(neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb): Not exist!
Copy your neuware package(neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb) into the directory of ffmpeg-mlu!
eg:cp -v /data/ftp/v1.5.0/neuware/neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb ./ffmpeg-mlu
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ cp -v /data/ftp/v1.5.0/neuware/neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb ./ffmpeg-mlu
'/data/ftp/v1.5.0/neuware/neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb' -> './ffmpeg-mlu/neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb'
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ./build-image-ffmpeg-mlu.sh
Directory(ffmpeg-mlu): Exists!
File(neuware-mlu270-1.5.0-1_Ubuntu16.04_amd64.deb): Exists!
====================== build image ======================
Sending build context to Docker daemon  92.31MB
Step 1/11 : FROM ubuntu:16.04
 ---> c522ac0d6194
Step 2/11 : MAINTAINER <Cambricon, Inc.>
 ---> Using cache
 ---> b00336dc9e2e
Step 3/11 : WORKDIR /root/ffmpeg-mlu/
 ---> Running in e05d71612dc5
Removing intermediate container e05d71612dc5
 ---> 55ad95055392
Step 4/11 : ARG neuware_version=neuware-mlu270-1.4.0
 ---> Running in bd321ecbcc0e
Removing intermediate container bd321ecbcc0e
 ---> 1e2a10c18314
Step 5/11 : ARG neuware_package=${neuware_version}-1_Ubuntu16.04_amd64.deb
 ---> Running in 58fea76bf995
Removing intermediate container 58fea76bf995
 ---> 689da717fb25
Step 6/11 : ARG mlu_platform=MLU270
 ---> Running in 0bf60054b265
Removing intermediate container 0bf60054b265
 ---> 006797238d92
Step 7/11 : RUN echo -e 'nameserver 114.114.114.114' > /etc/resolv.conf
 ---> Running in 3bb5d80434b2
Removing intermediate container 3bb5d80434b2
 ---> 48684c752843
Step 8/11 : RUN echo deb http://mirrors.aliyun.com/ubuntu/ xenial main restricted  > /etc/apt/sources.list &&     echo deb-src http://mirrors.aliyun.comltiverse universe >> /etc/apt/sources.list &&     echo deb http://mirrors.aliyun.com/ubuntu/ xenial-updates main restricted >> /etc/apt/sources.list && .com/ubuntu/ xenial universe >> /etc/apt/sources.list &&     echo deb http://mirrors.aliyun.com/ubuntu/ xenial-updates universe >> /etc/apt/sources.listiyun.com/ubuntu/ xenial multiverse >> /etc/apt/sources.list &&     echo deb http://mirrors.aliyun.com/ubuntu/ xenial-updates multiverse >> /etc/apt/sourrrors.aliyun.com/ubuntu/ xenial-backports main restricted universe multiverse >> /etc/apt/sources.list &&     echo deb http://mirrors.aliyun.com/ubuntu/> /etc/apt/sources.list &&     echo deb http://mirrors.aliyun.com/ubuntu/ xenial-security universe  >> /etc/apt/sources.list &&     echo deb http://mirrrity multiverse >> /etc/apt/sources.list &&     apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends             curl git ertificates nasm yasm             openssh-server             net-tools &&     apt-get clean &&     apt-get update --fix-missing &&     rm -rf /var/lib/a
 ---> Running in c25f69826365
Get:1 http://mirrors.aliyun.com/ubuntu xenial InRelease [247 kB]
......
Get:20 http://mirrors.aliyun.com/ubuntu xenial-security/restricted amd64 Packages [15.9 kB]
Get:21 http://mirrors.aliyun.com/ubuntu xenial-security/universe amd64 Packages [950 kB]
Get:22 http://mirrors.aliyun.com/ubuntu xenial-security/multiverse amd64 Packages [9253 B]
Fetched 30.0 MB in 27s (1082 kB/s)
Reading package lists...
Reading package lists...
Building dependency tree...
Reading state information...
Calculating upgrade...
The following packages will be upgraded:
  base-files libc-bin libc6 libpam-modules libpam-modules-bin libpam-runtime
  libpam0g libseccomp2 libsystemd0 libudev1 multiarch-support systemd
  systemd-sysv
13 upgraded, 0 newly installed, 0 to remove and 0 not upgraded.
Need to get 7757 kB of archives.
After this operation, 35.8 kB of additional disk space will be used.
Get:1 http://mirrors.aliyun.com/ubuntu xenial-updates/main amd64 base-files amd64 9.4ubuntu4.13 [61.8 kB]
......
......
Get:13 http://mirrors.aliyun.com/ubuntu xenial-updates/main amd64 multiarch-support amd64 2.23-0ubuntu11.2 [6828 B]
debconf: delaying package configuration, since apt-utils is not installed
Fetched 7757 kB in 5s (1366 kB/s)
(Reading database ... 4781 files and directories currently installed.)
Preparing to unpack .../base-files_9.4ubuntu4.13_amd64.deb ...
Unpacking base-files (9.4ubuntu4.13) over (9.4ubuntu4.11) ...
Setting up base-files (9.4ubuntu4.13) ...
Installing new version of config file /etc/issue ...
Installing new version of config file /etc/issue.net ...
Installing new version of config file /etc/lsb-release ...
Installing new version of config file /etc/update-motd.d/50-motd-news ...
Removing obsolete conffile /etc/default/motd-news ...
(Reading database ... 4780 files and directories currently installed.)
Preparing to unpack .../libc6_2.23-0ubuntu11.2_amd64.deb ...
debconf: unable to initialize frontend: Dialog
debconf: (TERM is not set, so the dialog frontend is not usable.)
debconf: falling back to frontend: Readline
debconf: unable to initialize frontend: Readline
......
......
STRIP   ffprobe
STRIP   ffmpeg
Removing intermediate container de323b70f095
 ---> 9c49aa72c4ca
Step 11/11 : WORKDIR /root/ffmpeg-mlu/ffmpeg/build
 ---> Running in 36ef5db65ba3
Removing intermediate container 36ef5db65ba3
 ---> 3c0e7a2fbbc5
Successfully built 3c0e7a2fbbc5
Successfully tagged ubuntu16.04_ffmpeg-mlu:v1.5.0
====================== save image ======================
-rw------- 1 cam cam 1590864384 10月 14 17:37 ubuntu16.04_ffmpeg-mlu-v1.5.0.tar.gz
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ls -lh
total 1.5G
-rwxrwxr-x 1 cam cam 2.3K 10月 14 16:44 build-image-ffmpeg-mlu.sh
drwxrwxr-x 5 cam cam 4.0K 10月 14 17:37 ffmpeg-mlu
-rwxrwxr-- 1 cam cam  690 10月 14 16:01 load-image-ffmpeg.sh
-rw-rw-r-- 1 cam cam 7.6K 10月 14 17:30 README.md
-rwxrwxr-x 1 cam cam 1.6K 10月 14 15:59 run-container-ffmpeg.sh
-rw------- 1 cam cam 1.5G 10月 14 17:37 ubuntu16.04_ffmpeg-mlu-v1.5.0.tar.gz
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$
```

## Load ##

```bash
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ./load-image-ffmpeg.sh
0
ubuntu16.04_ffmpeg-mlu:v1.5.0
The image(ubuntu16.04_ffmpeg-mlu:v1.5.0) is not loaded and is loading......
ce5739886d50: Loading layer [==================================================>]   2.56kB/2.56kB
8d0d6236af3a: Loading layer [==================================================>]  314.6MB/314.6MB
e0bddec44d27: Loading layer [==================================================>]  92.31MB/92.31MB
b946486c480d: Loading layer [==================================================>]  1.054GB/1.054GB
Loaded image: ubuntu16.04_ffmpeg-mlu:v1.5.0
The image(ubuntu16.04_ffmpeg-mlu:v1.5.0) information:
REPOSITORY                                              TAG                 IMAGE ID            CREATED             SIZE
ubuntu16.04_ffmpeg-mlu                                  v1.5.0              3c0e7a2fbbc5        14 minutes ago      1.57GB
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$
```

## Run ##

```bash
cam@cam-3630:/data/docker/ffmpeg-mlu/build-image-ffmpeg-mlu$ ./run-container-ffmpeg.sh
0
container-ffmpeg-mlu-v1.5.0
root@cam-3630:~/ffmpeg-mlu/ffmpeg/build# cd ../../
root@cam-3630:~/ffmpeg-mlu# ls
CHANGELOG.md  LICENSE  README.md  README_cn.md  dockerfile  ffmpeg  ffmpeg4.2_mlu.patch  mlu_op  tools
root@cam-3630:~/ffmpeg-mlu#
```
