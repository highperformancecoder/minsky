#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky, findObject
minsky.load('$here/test/testEq.mky')
# DataOp not currently supported, so delete it
findObject('DataOp')
minsky.canvas.deleteItem()
minsky.matlab('testEq.m')
EOF

python3 input.py
if test $? -ne 0; then fail; fi

# testEq has something of everything, but is not an executable model

diff -w $here/test/testEq.m testEq.m
if test $? -ne 0; then fail; fi

pass
