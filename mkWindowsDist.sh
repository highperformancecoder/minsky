version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
pushd gui-js
export NX_DAEMON=false # prevent hang in next step

# don't use the all in one packager script, as code signing is stuffed
#npm run export:package:windows
npm run build:web
npm run build:electron

rm -rf dist/executables

# 1) build win-unpacked only
npx electron-builder --win dir --x64 --config.directories.output=dist/executables

# 2) sign payload that will be embedded in installer
find dist/executables/win-unpacked -type f \( -iname "*.exe" -o -iname "*.node" \) -exec ../sign.sh {} \;

# 3) build NSIS installer from signed prepackaged directory
npx electron-builder --win nsis --x64 \
  --prepackaged dist/executables/win-unpacked \
  --config.directories.output=dist/executables

if [ -f dynamic_libraries/libravel.dll ]; then
    name=ravel-with-plugin
    # can't figure out how to set artifactName in beforePackHook.js
    mv dist/executables/ravel-$version.exe dist/executables/$name-$version.exe
else
    name=ravel
fi

../sign.sh dist/executables/$name-$version.exe
echo cp dist/executables/$name-$version.exe /tmp
cp dist/executables/$name-$version.exe /tmp
popd
