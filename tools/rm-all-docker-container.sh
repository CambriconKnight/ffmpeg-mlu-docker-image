#/bin/bash
set -e

echo "================================================="
#查看所有容器
docker ps -a

echo "================================================="
#stop停止所有容器
docker stop $(docker ps -a -q)
#remove删除所有容器
docker rm $(docker ps -a -q)
#快速删除docker无用容器，用在生产环境请谨慎操作，以免误删有用的容器；
#docker ps -a|grep 'Exited'|cut -d' ' -f1|xargs -I {} docker rm {}
echo "================================================="
#查看所有容器
docker ps -a
