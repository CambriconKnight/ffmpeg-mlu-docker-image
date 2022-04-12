#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     build-srs.sh
# UpdateDate:   2022/04/08
# Description:  Build SRS.
# Example:      ./build-srs.sh
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
#Build SRS from source:
cd srs/trunk && ./configure && make && ./objs/srs -c conf/srs.conf