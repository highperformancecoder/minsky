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
rm -rf $name
mkdir $name
mv linux-unpacked $name/app
rm $name/app/resources/node-addons/*.node
popd
popd
cp au.com.hpcoders.{Minsky.{desktop,appdata.xml},{Minsky,Ravelation}-mime.xml} gui-js/dist/executables/$name/
cp gui-tk/icons/MinskyLogo.svg gui-js/dist/executables/$name/au.com.hpcoders.Minsky.svg
pushd gui-js/dist/executables/
tar zcvf /tmp/$name.tar.gz $name
