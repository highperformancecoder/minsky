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
minsky.load $here/test/testInitDA.mky
# dimensionalAnalysis should throw
if [catch minsky.dimensionalAnalysis] tcl_exit 0
tcl_exit 1
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
