#/bin/bash
set -e

#Version
VERSION="v1.5.0"

if [[ $# -eq 1 ]];then
    VERSION=$1
fi
#Image name
MY_IMAGES="ubuntu16.04_ffmpeg-mlu:$VERSION"
#Docker container name
MY_CONTAINER="container-ffmpeg-mlu-$VERSION"

#####Modify according to your development environment#####
#SDK path on the host
PATH_WORKSPACE_HOST="/data/ftp"
#Work path on the docker container
PATH_WORKSPACE_DOCKER="/home/ftp"
#Work2 path on the host
PATH_WORKSPACE2_HOST="$PWD/../"
#Work2 path on the docker container
PATH_WORKSPACE2_DOCKER="/home/cam"
#Datasets path on the host
PATH_DATASETS_HOST="/data/datasets"
#Datasets path on the docker container
PATH_DATASETS_DOCKER="/data/datasets"
#Models path on the host
PATH_MODELS_HOST="/data/models"
#Models path on the docker container
PATH_MODELS_DOCKER="/data/models"
#Pro path on the host
#PATH_PRO_HOST="/data/pro"
#Pro path on the docker container
#PATH_PRO_DOCKER="/data/pro"

##########################################################

num=`sudo docker ps -a|grep -w "$MY_CONTAINER$"|wc -l`

echo $num
echo $MY_CONTAINER

if [ 0 -eq $num ];then
    #sudo xhost +
    sudo docker run -e DISPLAY=unix$DISPLAY --privileged=true \
        --net=host --pid=host -v /tmp/.X11-unix:/tmp/.X11-unix \
        -it -v $PATH_WORKSPACE_HOST:$PATH_WORKSPACE_DOCKER \
        -it -v $PATH_WORKSPACE2_HOST:$PATH_WORKSPACE2_DOCKER \
        -it -v $PATH_DATASETS_HOST:$PATH_DATASETS_DOCKER \
        -it -v $PATH_MODELS_HOST:$PATH_MODELS_DOCKER \
        --name $MY_CONTAINER $MY_IMAGES /bin/bash
else
    sudo docker start $MY_CONTAINER
    sudo docker exec -ti $MY_CONTAINER /bin/bash
fi
