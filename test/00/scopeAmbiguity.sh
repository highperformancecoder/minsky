#! /bin/sh

here=`pwd`
if test $? -ne 0; then exit 2; fi
tmp=/tmp/$$
mkdir $tmp
if test $? -ne 0; then exit 2; fi
cd $tmp
if test $? -ne 0; then exit 2; fi

fail()
{
    echo "FAILED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 1
}

pass()
{
    echo "PASSED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 0
}

trap "fail" 1 2 3 15
cat >input.tcl <<EOF
minsky.load $here/test/scopeAmbiguityTest.mky
minsky.latex scopeAmbiguityTest.tex 0
minsky.matlab scopeAmbiguityTest.m
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi
diff scopeAmbiguityTest.tex $here/test/scopeAmbiguityTest.tex
if [ $? -ne 0 ]; then fail; fi
diff scopeAmbiguityTest.m $here/test/scopeAmbiguityTest.m
if [ $? -ne 0 ]; then fail; fi

pass
