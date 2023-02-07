# Place the path to your certificate store in environment variable
# WINDOWS_SIGN_CERTIFICATE_NAME and the password to open it at
# WINDOWS_SIGN_TOKEN_PASSWORD

version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
pushd gui-js
if [ -f dynamic_libraries/libravel.dll ]; then
    name=ravel
else
    name=minsky
fi
npm run export:package:windows
java -jar $HOME/usr/bin/jsign-4.1.jar --keystore $WINDOWS_SIGN_CERTIFICATE_NAME --storetype PKCS12 --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" dist/executables/$name-$version.exe
popd
