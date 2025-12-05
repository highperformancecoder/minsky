#!/bin/bash

if [ $# -eq 0 ]; then
    project=minsky
else
    project=$1
fi
    
rm *.log
for i in Dockerfile-*[^~]; do
    case $i in
        Dockerfile-debian) versions="12 13";;
        Dockerfile-ubuntu) versions="22.04 24.04 25.04";;
        Dockerfile-fedora) versions="41 42";;
        Dockerfile-leap) versions="15.4 15.5 15.6 16.0";;
        *) versions=default;;
    esac
    for version in $versions; do
        if docker build --label obsSmokeTest --network=host --build-arg project=$project --build-arg version=$version --pull --no-cache -f $i .; then
            echo "$i-$version PASSED" >$i-$version.log
        else
            echo "$i-$version FAILED" >$i-$version.log
        fi &
    done
done
wait
docker container prune -f --filter label=obsSmokeTest
cat *.log
# check if any child process failed, and emit an appropriate status code
grep FAILED *.log &>/dev/null
exit $[!$?]
