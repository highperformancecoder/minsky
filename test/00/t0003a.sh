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

cp $here/test/testEq.mky .
$here/GUI/minsky $here/test/testEq.tcl
if test $? -ne 0; then fail; fi


diff -w $here/test/testEqPrototype.tex testEq.tex
if test $? -ne 0; then fail; fi

diff -w $here/test/testEqWrappedPrototype.tex testEqWrapped.tex
if test $? -ne 0; then fail; fi

pass
