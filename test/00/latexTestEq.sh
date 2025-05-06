#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky

minsky.load('testEq.mky')
minsky.latex('testEq.tex', False)
minsky.latex('testEqWrapped.tex', True)
EOF
cp $here/test/testEq.mky .
python3 input.py
if test $? -ne 0; then fail; fi


diff -w $here/test/testEqPrototype.tex testEq.tex
if test $? -ne 0; then fail; fi

diff -w $here/test/testEqWrappedPrototype.tex testEqWrapped.tex
if test $? -ne 0; then fail; fi

pass
