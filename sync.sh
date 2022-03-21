#!/bin/bash
set -e
# 0. Source env
source ./env.sh
#################### main ####################
# 1. check
if [ ! -d "$PATH_WORK" ];then
    mkdir -p $PATH_WORK
else
    echo "Directory($PATH_WORK): Exists!"
fi
#cp -rvf ./dependent_files/*.tar.gz ./$PATH_WORK
cp -rvf ./dependent_files/*.deb ./$PATH_WORK
#cp -rvf ./dependent_files/*.cambricon ./$PATH_WORK
