#!/bin/bash

# creates a Mac .pkg installer. 

# This script is intended to be called from make mac-dist target

MAC_DIST_DIR=minsky.app/Contents/MacOS
version=`cut -f3 -d' ' minskyVersion.h|head -1|tr -d '"'`
if [ $version = '"unknown"' ]; then
    version=0.0.0.0
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
}

cp gui-tk/minsky $MAC_DIST_DIR
rewrite_dylibs $MAC_DIST_DIR/minsky

# TkTable.dylib seems to be its own little snowflake :)
cp /usr/local/lib/libTktable2.11.dylib $MAC_DIST_DIR
rewrite_dylibs $MAC_DIST_DIR/libTktable2.11.dylib
install_name_tool -change libTktable2.11.dylib @executable_path/libTktable2.11.dylib $MAC_DIST_DIR/minsky

# determine location of tcl library from tclsh - make sure the correct
# tclsh is in your path
eval `tclsh echo_tcl_lib.tcl`

# copy the fontconfig info
mkdir -p $MAC_DIST_DIR/../Resources/fontconfig
cp -r /opt/local/etc/fonts/* $MAC_DIST_DIR/../Resources/fontconfig

echo "$TCL_LIB $TK_LIB"
mkdir -p $MAC_DIST_DIR/library
rm -rf $MAC_DIST_DIR/library/{tcl,tk}
cp -r $TCL_LIB $MAC_DIST_DIR/library/tcl
cp -r $TK_LIB $MAC_DIST_DIR/library/tk

#copy toplevel tcl scripts
    cp gui-tk/*.tcl $MAC_DIST_DIR
#copy library scripts 
    cp -r gui-tk/library $MAC_DIST_DIR
    cp -r gui-tk/icons $MAC_DIST_DIR
    cp gui-tk/accountingRules $MAC_DIST_DIR
    pkgbuild --root minsky.app --install-location /Applications/Minsky.app --identifier Minsky Minsky.$version-mac-dist.pkg
