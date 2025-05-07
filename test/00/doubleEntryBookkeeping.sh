#! /bin/sh

here=`pwd`
. $here/test/common-test.sh
# double entry bookkeeping tests

cat >input.py <<EOF
from pyminsky import minsky

minsky.canvas.addGodley()
item=minsky.canvas.itemFocus
minsky.canvas.getItemAt(item.x(), item.y())
item.table.clear()
item.table.resize(4, 4)
item.table.setDEmode(True)
item.setCell(0, 1, 's1')
item.setCell(0, 2, 's2')
item.setCell(0, 3, 's3')
item.setCell(1, 0, "Initial Conditions")
item.setCell(1, 1, '10')
item.setCell(1, 3, '10')
item.setCell(2, 1, 'a')
item.setCell(2, 2, 'b')
item.setCell(2, 3, 'a')

item.table._assetClass(1, 'asset')
item.table._assetClass(2, 'liability')
item.table._assetClass(3, 'liability')
assert minsky.canvas.item.table.rowSum(1)=='0'
assert minsky.canvas.item.table.rowSum(2)=="-b"
assert minsky.canvas.item.table.getVariables()()==['a', 'b']
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
