#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky, findObject

minsky.load('$here/examples/exponentialGrowth.mky')
minsky.numBackups(3)
item=findObject("IntOp")
for i in range(10):
  item.description('y'+str(i))
  minsky.save('foo.mky')
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

if [ `grep "<name>y" foo.mky*|wc -l` -ne 4 ]; then fail; fi

for i in 3 2 1; do
    if ! grep "<name>y$[9-i]" "foo.mky;$i" >/dev/null; then fail; fi
done

pass
