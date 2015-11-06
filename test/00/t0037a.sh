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

# checks for the presence of debug print statements to stdout in the TCL files,
# which cause problems in the Windows release build
numDebugPrintStmts=`grep puts $here/GUI/*.tcl|csvprintf -s ' ' "%0\\\$d\n"|grep 3|wc -l`

#There should be precisely 1 statement, in the definition of tout
if [ $numDebugPrintStmts -gt 1 ]; then fail; fi

pass
