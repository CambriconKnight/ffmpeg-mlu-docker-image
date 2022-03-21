#/bin/bash
set -e
if [[ $# -eq 0 ]];then
    echo -e "\033[1;33m Usage: $0 [version] \033[0m"
    exit -1
fi
git branch
#NameBranch="v1.7.602"
NameBranch=$1
#eg: git checkout -b v1.7.602
git checkout -b $NameBranch
#eg: git push origin v1.7.602
git push origin $NameBranch
git branch -a
git checkout master
git branch -a
