#/bin/bash
set -e

# 1.rm
cd /home/share/test/api/transcode
make clean
rm -vf ./transcode_*.mp4
ls -la
