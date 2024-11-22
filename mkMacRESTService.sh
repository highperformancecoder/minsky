#!/opt/local/bin/bash

# creates a Mac .pkg installer. 

# This script is intended to be called from make mac-dist target

# ensure ~/usr/bin overrides every other TCL installation
PATH=~/usr/bin:$PATH

# check that EcoLab and Tk has been built for extracting a quartz context
if ! nm ecolab/lib/libecolab.a|c++filt|grep NSContext::NSContext|grep T; then
    echo "Rebuild EcoLab with MAC_OSX_TK=1"
    exit 1
fi

# check that the keychain has been unlocked
if security show-keychain-info|grep "User interaction is not allowed."; then
    echo "Unlock the keychain with security unlock-keychain"
    exit 1
fi

MAC_DIST_DIR=gui-js/build
version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
if [ $version = '"unknown"' ]; then
    version=0.0.0.0
fi

# for some reason arch misreports x64 as i386 on Intel Macs
if [ `arch` = i386 ]; then
    target=gui-js/dist/ravel-$version-x64.dmg
else
    target=gui-js/dist/ravel-$version-`arch`.dmg
fi

# determine release or beta depending on the number of fields separated by '-' in the version string
numFields=`echo $version|tr - ' '|wc -w`
if [ $numFields -le 1 ]; then
    productName=Ravel
else
    productName=RavelBeta
fi

rewrite_dylib()
{
    local dylib=$1
    local target=$2
    cp -f $dylib $MAC_DIST_DIR
    chmod u+rw $MAC_DIST_DIR/${dylib##*/}
    rewrite_dylibs $MAC_DIST_DIR/${dylib##*/}
    echo "install_name_tool -change $dylib @loader_path/${dylib##*/} $target"
    install_name_tool -change $dylib @loader_path/${dylib##*/} $target
}

rewrite_dylibs()
{
    local target=$1
    echo "rewrite_dylibs $target"
    otool -L $target|grep opt/|cut -f1 -d' '|while read dylib; do
        # avoid infinite loops
        if [ "${dylib##*/}" == "${target##*/}" ]; then 
            install_name_tool -change $dylib @loader_path/${dylib##*/} $target
            continue
        else
            rewrite_dylib $dylib $target
        fi
    done
    otool -L $target|grep usr/local|cut -f1 -d' '|while read dylib; do
        rewrite_dylib $dylib $target
    done
    
    install_name_tool -id @loader_path/${target##*/} $target
}

rm -rf $MAC_DIST_DIR/*.dylib
mkdir -p $MAC_DIST_DIR

rewrite_dylibs $MAC_DIST_DIR/minskyRESTService.node

pushd gui-js
# libravel.so expects certain dylibs in node-addons for legacy
# reasons. Once the Ravel application has moved on, then we can drop
# this, and update the expected location in the Ravel Makefile.
# make a copy here, as creating symlinks in electron-builder seems nigh on impossible
rm -rf node-addons
mkdir node-addons
pushd node-addons
ln -sf ../build/lib*.dylib .
popd
export NX_DAEMON=false # prevent hangs in the following steps
#npm run export:package:mac
npm run build:web
npm run build:electron
npx electron-builder
popd

# npm run export step actually does the code signing. Following is for reference
#codesign -s "Developer ID Application" --options runtime --timestamp --deep $target

# notarytool is introduced from Big Sur onwards, altool has been deprecated.
if [ `sw_vers|grep ProductVersion|tr -s '\t'|cut -f2|cut -f1 -d.` -lt 11 ]; then
   xcrun altool --notarize-app --primary-bundle-id Minsky --username apple@hpcoders.com.au --password "@keychain:Minsky" --file $target
else
    # Note: use xcrun notarytool store-credentials --apple-id \
    # apple@hpcoders.com.au --team-id 3J798GK5A7 --password "...", and
    # specify "NotaryTool" as the profile id.  Password can be
    # generated at https://appleid.apple.com. You will need to
    # generate a new "application password" every time you set up a
    # new machine.
    xcrun notarytool submit $target  --keychain-profile NotaryTool  --wait
fi
   
xcrun stapler staple $target
if [ $? -ne 0 ]; then
    echo "Manually staple with:"
    echo "xcrun stapler staple $target"
fi

exit

