#!/bin/bash

# creates a Mac .pkg installer. 

# This script is intended to be called from make mac-dist target

# ensure ~/usr/bin overrides every other TCL installation
PATH=~/usr/bin:$PATH

# check that EcoLab and Tk has been built for extracting a quartz context
if ! nm ecolab/lib/libecolab.a|c++filt|grep NSContext::NSContext|grep T; then
    echo "Rebuild EcoLab with MAC_OSX_TK=1"
    exit 1
fi

MAC_DIST_DIR=gui-js/node-addons
version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
if [ $version = '"unknown"' ]; then
    version=0.0.0.0
fi

target=gui-js/dist/executables/minsky-$version.dmg

# determine release or beta depending on the number of fields separated by '-' in the version string
numFields=`echo $version|tr - ' '|wc -w`
if [ $numFields -le 1 ]; then
    productName=Minsky
else
    productName=MinskyBeta
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

# due to the presence of -isystem /usr/local/lib, which is needed for other idiocies, libjson_spirit is not correctly rewritten by the above
#rewrite_dylib /usr/local/lib/libjson_spirit.dylib $MAC_DIST_DIR/minskyRESTService.node
#install_name_tool -change libjson_spirit.dylib @loader_path/libjson_spirit.dylib $MAC_DIST_DIR/minskyRESTService.node

pushd gui-js
npm run export:package:mac
popd

# notarytool is introduced from Big Sur onwards, altool has been deprecated.
if [ `sw_vers|grep ProductVersion|cut -f2|cut -f1 -d.` -lt 11 ]; then
   xcrun altool --notarize-app --primary-bundle-id Minsky --username apple@hpcoders.com.au --password "@keychain:Minsky" --file $target
else
    # Note: use xcrun notarytool store-credentials --apple-id apple@hpcoders.com.au --team-id 3J798GK5A7 --password "...", and specify "NotaryTool" as the profile id.
    xcrun notarytool submit $target  --keychain-profile NotaryTool  --wait
fi
   
xcrun stapler staple $target
if [ $? -ne 0 ]; then
    echo "Manually staple with:"
    echo "xcrun stapler staple $target"
fi

exit

