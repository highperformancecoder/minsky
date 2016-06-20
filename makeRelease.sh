echo "old version=`git describe`"
echo '#define MINSKY_VERSION "'$1'"' >minskyVersion.h
git commit -a -m "Release $1"
git tag -a -m "" $1
