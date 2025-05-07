#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky
minsky.load('$here/test/testInitDA.mky')
# dimensionalAnalysis should throw
try:
    minsky.dimensionalAnalysis()
except:
    exit(0)
exit(1)
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
