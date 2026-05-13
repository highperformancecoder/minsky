#!/bin/bash

apiUrl=https://minskybe-x7dj1.sevalla.app
version=`git describe|sed -e 's/-.*//'`
releaseDate=`git log -1 --format=%cs --date=iso`
arch=`arch`
if [ $arch = 'i386' ]; then arch=x64; fi
os=`uname`

if [ $os = Darwin ]; then 
    python3 ~/usr/bin/upload_assets.py --api-url $apiUrl --product minsky --version $version --os macos --arch $arch --release-date $releaseDate --sign ~/minsky-signing.key gui-js/dist/ravel-$version-$arch.dmg
else #linux, but assume we're running this in a MXE build directory
    python3 ~/usr/bin/upload_assets.py --api-url $apiUrl --product minsky --version $version --os windows --arch $arch --release-date $releaseDate --sign ~/minsky-signing.key gui-js/dist/executables/ravel-$version.exe
fi
