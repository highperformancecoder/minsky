#! /bin/sh

here=`pwd`
. $here/test/common-test.sh
# test of undo/redo

cat >input.py <<EOF
from pyminsky import minsky, findObject

minsky.load('$here/examples/1Free.mky')
minsky.save('checkpoint1.mky')
minsky.checkPushHistory

findObject('Operation:multiply')
minsky.canvas.deleteItem()
minsky.checkPushHistory
minsky.save('checkpoint2.mky')
minsky.undo(1)
minsky.save('checkpoint3.mky')
minsky.undo(-1)
minsky.save('checkpoint4.mky')
EOF

python3 input.py
if test $? -ne 0; then fail; fi

$here/test/cmpFp checkpoint1.mky checkpoint3.mky
if test $? -ne 0; then fail; fi

$here/test/cmpFp checkpoint2.mky  checkpoint4.mky
if test $? -ne 0; then fail; fi


pass
