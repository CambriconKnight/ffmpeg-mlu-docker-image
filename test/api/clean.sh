#/bin/bash
set -e

# 1.rm
cd ./transcode
make clean
rm -vf ./transcode_*.mp4
ls -la
