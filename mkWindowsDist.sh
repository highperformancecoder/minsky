# Place the path to your certificate store in environment variable
# WINDOWS_SIGN_CERTIFICATE_NAME and the password to open it at
# WINDOWS_SIGN_TOKEN_PASSWORD

version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
pushd gui-js
npm run export:package:windows

if [ -f dynamic_libraries/libravel.dll ]; then
    name=ravel-with-plugin
    # can't figure out how to set artifactName in beforePackHook.js
    mv dist/executables/ravel-$version.exe dist/executables/$name-$version.exe
else
    name=ravel
fi
sudo java -jar $HOME/usr/bin/jsign-6.0.jar --keystore $HOME/usr/etc/SafeNet.cfg --storetype ETOKEN --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" -t http://timestamp.sectigo.com dist/executables/$name-$version.exe
echo cp dist/executables/$name-$version.exe /tmp
cp dist/executables/$name-$version.exe /tmp
popd
