#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.load('$here/test/testEq.mky')
minsky.canvas.recentre()
minsky.equationDisplay.renderToPS('testEq.ps')
assert minsky.equationDisplay.width()>0
assert minsky.equationDisplay.height()>0
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
