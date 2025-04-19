#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0, '$here')
from pyminsky import minsky

minsky.load('constantCopyProblem.mky')
minsky.step()
assert minsky.variableValues[':b'].value(0)==12
EOF

cp $here/test/constantCopyProblem* .

python3 input.py
if test $? -ne 0; then fail; fi

pass
