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

$here/RESTService/minsky-RESTService >output <<EOF
/minsky/variableValues/@elem/constant:one
EOF
if [ $? -ne 0 ]; then fail; fi

cat >reference <<EOF
/minsky/variableValues/@elem/constant:one=>{"first":"constant:one","second":{"csvDialog":{"colWidth":50,"flashNameRow":false,"spec":{"columnar":false,"dataColOffset":0,"dataRowOffset":0,"decSeparator":".","dimensionCols":[],"dimensionNames":[],"dimensions":[],"duplicateKeyAction":"throwException","escape":"\\\\","headerRow":0,"horizontalDimName":"?","horizontalDimension":{"type":"string","units":""},"mergeDelimiters":false,"missingValue":nan,"quote":"\"","separator":","},"url":"","xoffs":80},"godleyOverridden":false,"init":"1","name":"constant:one","sliderVisible":false,"tensorInit":{},"units":[],"unitsCached":false}}
EOF

diff -q -w output reference
if [ $? -ne 0 ]; then fail; fi

pass
