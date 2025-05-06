#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky

minsky.load('$here/test/scopeAmbiguityTest.mky')
minsky.latex('scopeAmbiguityTest.tex', False)
minsky.matlab('scopeAmbiguityTest.m')
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi
diff scopeAmbiguityTest.tex $here/test/scopeAmbiguityTest.tex
if [ $? -ne 0 ]; then fail; fi
diff scopeAmbiguityTest.m $here/test/scopeAmbiguityTest.m
if [ $? -ne 0 ]; then fail; fi

pass
