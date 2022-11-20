#/bin/bash
set -e

# 1.rm
make clean
rm -vf ./*.yuv
rm -vf ./core.*
ls -la
