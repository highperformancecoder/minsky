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

MAC_DIST_DIR=minsky.app/Contents/MacOS
version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
if [ $version = '"unknown"' ]; then
    version=0.0.0.0
fi

# determine release or beta depending on the number of fields in the version
numFields=`echo $version|tr . ' '|wc -w`
if [ $numFields -le 2 ]; then
    productName=Minsky
else
    productName=MinskyBeta
fi

rewrite_dylibs()
{
    local target=$1
    echo "rewrite_dylibs $target"
    otool -L $target|grep opt/|cut -f1 -d' '|while read dylib; do
        # avoid infinite loops
        if [ "${dylib##*/}" == "${target##*/}" ]; then 
            install_name_tool -change $dylib @executable_path/${dylib##*/} $target
            continue
        else
            cp -f $dylib $MAC_DIST_DIR
            chmod u+rw $MAC_DIST_DIR/${dylib##*/}
            rewrite_dylibs $MAC_DIST_DIR/${dylib##*/}
            echo "install_name_tool -change $dylib @executable_path/${dylib##*/} $target"
            install_name_tool -change $dylib @executable_path/${dylib##*/} $target
        fi
    done
    otool -L $target|grep usr/local|cut -f1 -d' '|while read dylib; do
        cp -f $dylib $MAC_DIST_DIR
        chmod u+rw $MAC_DIST_DIR/${dylib##*/}
        rewrite_dylibs $MAC_DIST_DIR/${dylib##*/}
        install_name_tool -change $dylib @executable_path/${dylib##*/} $target
    done
    install_name_tool -id @executable_path/${target##*/} $target
}

cp gui-tk/minsky $MAC_DIST_DIR
rewrite_dylibs $MAC_DIST_DIR/minsky

# copy the fontconfig info
mkdir -p $MAC_DIST_DIR/../Resources/fontconfig
cp -r /opt/local/etc/fonts/* $MAC_DIST_DIR/../Resources/fontconfig

# TCL files etc in the executable directory causes grief with code signing, so move these to the lib directory
# minskyHome is automatically set to that directory
MINSKYHOME=$MAC_DIST_DIR/../lib/minsky
mkdir -p $MINSKYHOME

#copy toplevel tcl scripts
cp gui-tk/*.tcl $MINSKYHOME
#copy library scripts 
cp -r gui-tk/library $MINSKYHOME
cp -r gui-tk/icons $MINSKYHOME
cp gui-tk/accountingRules $MINSKYHOME
    
# determine location of tcl library 
eval `gui-tk/minsky echo_tcl_lib.tcl`

echo "$TCL_LIB $TK_LIB"
mkdir -p $MINSKYHOME/library
rm -rf $MINSKYHOME/library/{tcl,tk}
cp -r $TCL_LIB $MINSKYHOME/library/tcl
cp -r $TK_LIB $MINSKYHOME/library/tk

codesign -s "Developer ID Application" --deep --force --options runtime minsky.app
if [ $? -ne 0 ]; then
    echo "try running this script on the Mac GUI desktop, not ssh shell"
fi

# how to build and sign a .pkg file left for historical interest
#productbuild --root minsky.app /Applications/Minsky.app minsky.pkg
#productsign --sign "Developer ID Installer" minsky.pkg Minsky-$version-mac-dist.pkg
#if [ $? -ne 0 ]; then
#    echo "try running this script on the Mac GUI desktop, not ssh shell"
#fi

rm -f $productName-$version-mac-dist.dmg
hdiutil create -srcfolder minsky.app -volname Minsky -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 150M temp.dmg
hdiutil convert -format UDZO -imagekey zlib-level=9 -o $productName-$version-mac-dist.dmg temp.dmg 
rm -f temp.dmg
codesign -s "Developer ID Application" --options runtime $productName-$version-mac-dist.dmg
xcrun altool --notarize-app --primary-bundle-id Minsky --username apple@hpcoders.com.au --password "@keychain:Minsky" --file $productName-$version-mac-dist.dmg
# check using xcrun altool --notarization-history 0 -u apple@hpcoders.com.au -p "@keychain:Minsky"
echo "Sleeping for a bit for software to be notarised"
sleep 60
xcrun stapler staple $productName-$version-mac-dist.dmg
if [ $? -ne 0 ]; then
    echo "Manually staple with:"
    echo "xcrun stapler staple $productName-$version-mac-dist.dmg"
fi
