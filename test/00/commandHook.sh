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

# This test exercises the commandHook functionality in RESTService

$here/RESTService/minsky-RESTService -batch >output <<EOF
/minsky/pushHistory
/minsky/edited
/minsky/nSteps
/minsky/edited
/minsky/nSteps 10
/minsky/edited
/minsky/load "$here/examples/GoodwinLinear02.mky"
/minsky/edited
/minsky/save "foo.mky"
/minsky/edited
/minsky/canvas/addOperation "add"
/minsky/edited
/minsky/save "foo.mky"
/minsky/edited
EOF
if [ $? -ne 0 ]; then fail; fi

cat >reference <<EOF
/minsky/pushHistory=>true
/minsky/edited=>false
/minsky/nSteps=>1
/minsky/edited=>false
/minsky/nSteps=>10
/minsky/edited=>true
/minsky/load=>null
/minsky/edited=>false
/minsky/save=>null
/minsky/edited=>false
/minsky/canvas/addOperation=>null
/minsky/edited=>true
/minsky/save=>null
/minsky/edited=>false
EOF

diff -q -w output reference
if [ $? -ne 0 ]; then fail; fi

pass
