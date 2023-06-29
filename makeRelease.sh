echo "old version=`git describe`"
echo '#define MINSKY_VERSION "'$1'"' >minskyVersion.h
echo 'export const version="'$1'";' >gui-js/libs/shared/src/lib/constants/version.ts
sed -i -e "s/\<!--version--\>.*\<\/string\>/\<!--version--\>\<string\>$1\<\/string\>/" minsky.app/Contents/Info.plist
sed -i -e "s/\"version\":.*,/\"version\":\"$1\",/" gui-js/package.json
sed -i -e "1,10s/\"version\":.*,/\"version\": \"$1\",/" gui-js/package-lock.json
git commit -a -S -m "Release $1"
git tag -s -m "" $1
