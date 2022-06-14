#!/bin/bash
set -e
# -------------------------------------------------------------------------------
# Filename:     install_cntoolkit.sh
# UpdateDate:   2022/06/08
# Description:  Install CNToolkit.
# Example:      ./install_cntoolkit.sh
# Depends:
#               CNToolkit(ftp://username@download.cambricon.com:8821/product/GJD/MLU270/1.7.604/Ubuntu16.04/CNToolkit/cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb)
# Notes:
# -------------------------------------------------------------------------------
#
#Font color
none="\033[0m"
green="\033[0;32m"
red="\033[0;31m"
yellow="\033[1;33m"
white="\033[1;37m"
# 1. Parameter configuration
CNToolkitPackageName="cntoolkit_1.7.5-1.ubuntu16.04_amd64.deb"
if [[ $# -ne 0 ]];then CNToolkitPackageName="${1}";fi
echo -e "${green}[Install CNToolkit(${CNToolkitPackageName})... ] ${none}"
CNToolkit_Version=`echo ${CNToolkitPackageName}|awk -F '-' '{print $1}'`
CNToolkit_Version_1=`echo ${CNToolkit_Version}|awk -F '_' '{print $1}'`
CNToolkit_Version_2=`echo ${CNToolkit_Version}|awk -F '_' '{print $2}'`
CNToolkit_Version="${CNToolkit_Version_1}-${CNToolkit_Version_2}"

# 2. Install CNToolkit
dpkg -i ./$CNToolkitPackageName \
&& dpkg -i /var/${CNToolkit_Version}/cndev*.deb \
&& dpkg -i /var/${CNToolkit_Version}/cnbin*.deb \
&& dpkg -i /var/${CNToolkit_Version}/cndrv*.deb \
&& dpkg -i /var/${CNToolkit_Version}/cnrt_*.deb \
&& dpkg -i /var/${CNToolkit_Version}/cncodec*.deb
echo -e "${green}[Install CNToolkit... Done] ${none}"
