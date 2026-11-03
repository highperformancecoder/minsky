#!/bin/bash

if [ $# -eq 0 ]; then
    project=minsky
else
    project=$1
fi
    
rm *.log

# run sequentially, using the cache on Github action runner
if [ -z "$GITHUB_ACTIONS" ]; then
    CACHE_FLAG="--no-cache"
    BG="&"
    echo "Running locally: --no-cache is ENABLED."
else
    CACHE_FLAG=""
    BG=
    echo "Running in GitHub Actions: --no-cache is DISABLED (relying on clean runner cache)."
fi

# Pre-pull base images sequentially first to avoid registry stampede/rate limits
for i in debian fedora opensuse/leap ubuntu; do
    case $i in
        debian) versions="12 13";;
        ubuntu) versions="24.04 25.04 25.10 26.04";;
        fedora) versions="41 42 43 44 rawhide";;
        opensuse/leap) versions="15.6 16.0";;
        *) versions=default;;
    esac
    for version in $versions; do
        docker pull $i:$version
    done
done
docker pull opensuse/tumbleweed 

smokeIt()
{
    project=$1
    version=$2
    dockerFile=$3
    flags=$4
    if docker build --label obsSmokeTest --network=host --build-arg project=$project --build-arg version=$version $flags -f $dockerFile .; then
            echo "$dockerFile-$version PASSED" >$dockerFile-$version.log
    else
        echo "$dockerFile-$version FAILED" >$dockerFile-$version.log
    fi
}

for i in Dockerfile-*[^~]; do
    case $i in
        Dockerfile-debian) versions="12 13";;
        Dockerfile-ubuntu) versions="24.04 25.04 25.10 26.04";;
        Dockerfile-fedora) versions="41 42 43 44";;
        Dockerfile-leap) versions="15.6 16.0";;
        *) versions=default;;
    esac
    for version in $versions; do
        if [ -z "$GITHUB_ACTIONS" ]; then
            smokeIt $project $version $i $CACHE_FLAG &
        else # do it sequentially on Github to avoit OOM killers
            smokeIt $project $version $i $CACHE_FLAG
        fi
    done
done
wait
docker container prune -f --filter label=obsSmokeTest
cat *.log
# check if any child process failed, and emit an appropriate status code
grep FAILED *.log &>/dev/null
exit $[!$?]
