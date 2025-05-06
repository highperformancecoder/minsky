#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky

minsky.load('$here/examples/1Free.mky')
assert minsky.model.defaultExtension()==".mky"

minsky.canvas.addRavel()
assert minsky.model.defaultExtension()==".rvl"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
