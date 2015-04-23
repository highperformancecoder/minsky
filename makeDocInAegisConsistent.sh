#!/bin/bash
rm -rf doc/minsky
(cd doc; sh makedoc.sh)

target=GUI/library/help
aegis -cp GUI/library
rm -rf $target/*
mkdir -p $target/minsky
find doc/minsky \( -name "*.html" -o -name "*.css" -o -name "*.png" \) -exec cp {} $target/minsky \;
cp -r -f doc/minsky.html $target
aegis -list -terse project_files|grep $target|while read nm; do 
    if [ ! -f $nm ]; then 
        aegis -cpu $nm 
        aegis -rm $nm 
    fi
done
find $target -exec aegis -nf {} \;
aegis -cpu -unch GUI/library

aegis -cp GUI/helpRefDb.tcl
perl makeRefDb.pl doc/minsky/labels.pl >GUI/helpRefDb.tcl
