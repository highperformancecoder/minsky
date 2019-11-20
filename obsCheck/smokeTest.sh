#!/bin/bash
rm *.log
for i in Dockerfile-*[^~]; do
    if docker build --network=host --pull -f $i .; then
        echo "$i PASSED" >$i.log
    else
        echo "$i FAILED" >$i.log
    fi &
done
wait
docker container prune -f
cat *.log
