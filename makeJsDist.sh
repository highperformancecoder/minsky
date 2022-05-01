fullversion=`git describe`
version=${fullversion%%-*}
extra=${fullversion##*-}
if [ $extra = $version ]; then
    name=minskyjs-$version
else
    name=minskyjs-$version-$extra
fi
pushd gui-js
npm run export:package:linux
pushd dist/executables/
mv linux-unpacked $name
rm $name/resources/node-addons/*.node
tar zcvf /tmp/$name.tar.gz $name
