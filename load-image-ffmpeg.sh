#/bin/bash
set -e

#Version
VERSION="v1.5.0"

if [[ $# -eq 1 ]];then
    VERSION=$1
fi
#Images name
FILENAME_IMAGES="ubuntu16.04_ffmpeg-mlu-$VERSION.tar.gz"
MY_IMAGES="ubuntu16.04_ffmpeg-mlu"

num=`sudo docker images | grep -w "$MY_IMAGES" | grep -w "$VERSION" | wc -l`
echo $num
echo $MY_IMAGES:$VERSION

if [ 0 -eq $num ];then
    echo "The image($MY_IMAGES:$VERSION) is not loaded and is loading......"
    #load image
    sudo docker load < $FILENAME_IMAGES
else
    echo "The image($MY_IMAGES:$VERSION) is already loaded!"
fi

#echo "All image information:"
#sudo docker images
echo "The image($MY_IMAGES:$VERSION) information:"
sudo docker images | grep -e "REPOSITORY" -e $MY_IMAGES
