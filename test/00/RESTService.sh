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

$here/RESTService/RESTService >output <<EOF
/minsky/variableValues/@elem/constant:one

EOF
if [ $? -ne 0 ]; then fail; fi

cat >reference <<EOF
cmd>/minsky/variableValues/@elem/constant:one
{"first":"constant:one","second":{"godleyOverridden":false,"init":"1","name":"constant:one","tensorInit":{"data":[],"dims":[]},"units":[],"unitsCached":false,"xVector":[]}}
cmd>
EOF

diff -q -w output reference
if [ $? -ne 0 ]; then fail; fi

pass
