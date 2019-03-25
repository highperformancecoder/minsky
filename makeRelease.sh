echo "old version=`git describe`"
echo '#define MINSKY_VERSION "'$1'"' >minskyVersion.h
sed -i -e "s/\<!--version--\>.*\<\/string\>/\<!--version--\>\<string\>$1\<\/string\>/" minsky.app/Contents/Info.plist
git commit -a -m "Release $1"
git tag -a -m "" $1
