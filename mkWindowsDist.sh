# Place the path to your certificate store in environment variable
# WINDOWS_SIGN_CERTIFICATE_NAME and the password to open it at
# WINDOWS_SIGN_TOKEN_PASSWORD

version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
pushd gui-js
export NX_DAEMON=false # prevent hang in next step
npm run export:package:windows

if [ -f dynamic_libraries/libravel.dll ]; then
    name=ravel-with-plugin
    # can't figure out how to set artifactName in beforePackHook.js
    mv dist/executables/ravel-$version.exe dist/executables/$name-$version.exe
else
    name=ravel
fi
#java -jar $HOME/usr/bin/jsign-4.1.jar --keystore $WINDOWS_SIGN_CERTIFICATE_NAME --storetype PKCS12 --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" dist/executables/$name-$version.exe
sudo java -jar $HOME/usr/bin/jsign-6.0.jar --storetype ETOKEN --keystore ~/usr/etc/SafeNet.cfg -t http://timestamp.sectigo.com --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" dist/executables/$name-$version.exe
echo cp dist/executables/$name-$version.exe /tmp
cp dist/executables/$name-$version.exe /tmp
popd
