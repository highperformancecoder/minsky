#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# checks this bit of code does not throw
cat >input.py <<EOF
from pyminsky import minsky
minsky.load('$here/test/ticket511.mky')
minsky.step()
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
