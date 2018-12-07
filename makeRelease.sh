echo "old version=`git describe`"
echo '#define MINSKY_VERSION "'$1'"' >minskyVersion.h
git commit -a -m "Release $1"
git tag -a -m "" $1
sed -i -e 's/\<!--version--\>.*\<\/string\>/\<!--version--\>$1\<\/string\>/' minsky.app/Contents/Info.plist
