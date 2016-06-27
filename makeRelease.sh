echo "old version=`git describe`"
git tag -a -m "" $1
echo '#define MINSKY_VERSION "'$1'"' >minskyVersion.h
