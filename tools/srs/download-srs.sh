#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     download-srs.sh
# UpdateDate:   2022/04/08
# Description:  download srs.
# Example:      ./download-srs.sh.sh
# Depends:
#               SRS(git clone -b 4.0release https://github.com/ossrs/srs.git)
#               SRS(git clone -b 4.0release https://gitee.com/ossrs/srs.git)
# Notes:
# -------------------------------------------------------------------------------
#Font color
none="\033[0m"
green="\033[0;32m"
red="\033[0;31m"
yellow="\033[1;33m"
white="\033[1;37m"
#ENV
PATH_WORK_TMP="srs"
VERSION_WORK="4.0release"
NAME_WORK_ALIGN="$PATH_WORK_TMP-$VERSION_WORK"
FILENAME_WORK_ALIGN="${NAME_WORK_ALIGN}.tar.gz"
#############################################################
# 1. Download
if [ ! -d "${PATH_WORK_TMP}" ];then
    echo -e "${green}git clone $PATH_WORK_TMP......${none}"
    git clone -b $VERSION_WORK https://gitee.com/ossrs/srs.git
else
    echo "Directory($PATH_WORK_TMP): Exists!"
fi

# del .git
pushd $PATH_WORK_TMP
find ../ -name ".git" | xargs rm -Rf
popd
