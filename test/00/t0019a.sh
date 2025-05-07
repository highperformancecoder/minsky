#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.load('$here/test/ticket265.mky')
minsky.reset()
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
