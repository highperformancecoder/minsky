#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky

minsky.load('$here/test/implicitTensorExample.mky')
minsky.nSteps(10)
minsky.step()
minsky.step()
assert minsky.t()>0
minsky.load('$here/test/derivativeNotDefined.mky')
minsky.nSteps(10)

thrown=False
try:
        minsky.step()
except:
        thrown=True
assert thrown
assert minsky.t()==0
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
