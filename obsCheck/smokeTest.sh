#!/bin/bash

if [ $# -eq 0 ]; then
    project=minsky
else
    project=$1
fi
    
rm *.log
status=0
for i in Dockerfile-*[^~]; do
    if docker build --network=host --build-arg project=$project --pull -f $i .; then
        echo "$i PASSED" >$i.log
    else
        echo "$i FAILED" >$i.log
        let $[status++]
    fi &
done
wait
docker container prune -f
cat *.log
exit $status
