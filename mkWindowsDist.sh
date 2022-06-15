version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
pushd gui-js
npm run export:package:windows
java -jar $HOME/usr/bin/jsign-4.1.jar --keystore $WINDOWS_SIGN_CERTIFICATE_NAME --storetype PKCS12 --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" dist/executables/minsky-$version.exe
popd
