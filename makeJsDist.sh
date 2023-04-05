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
mkdir $name
mv linux-unpacked $name/app
rm $name/app/resources/node-addons/*.node
popd
popd
cp minsky.desktop gui-js/dist/executables/$name/
cp gui-tk/icons/MinskyLogo.svg gui-js/dist/executables/$name/minsky.svg
pushd gui-js/dist/executables/
tar zcvf /tmp/$name.tar.gz $name
