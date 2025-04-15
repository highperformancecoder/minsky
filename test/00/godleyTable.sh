#! /bin/sh

here=`pwd`
. $here/test/common-test.sh
# basic godley table tests

cat >input.py <<EOF
import sys
sys.path.insert(0, '$here')
from pyminsky import minsky

minsky.canvas.addGodley()
item=minsky.canvas.itemFocus
minsky.canvas.getItemAt(item.x(), item.y())
item.table.clear()
assert minsky.canvas.item.table.rows()==0
assert minsky.canvas.item.table.cols()==0

# check insertRow/insertCol
item.table.insertRow(0)
item.table.insertCol(0)
assert minsky.canvas.item.table.rows()==1, "after insert"
assert minsky.canvas.item.table.cols()==1, "after insert"
item.setCell(0, 0, 'f11')
assert minsky.canvas.item.table.getCell(0, 0)=='f11'
item.table.insertRow(0)
item.table.insertCol(0)
assert minsky.canvas.item.table.rows()==2, "2nd insert"
assert minsky.canvas.item.table.cols()==2, "2nd insert"
minsky.canvas.item.table.getCell(1, 1)=="f11", "2nd insert"
item.table.insertRow(2)
item.table.insertCol(2)
assert minsky.canvas.item.table.rows()==3
assert minsky.canvas.item.table.cols()==3
assert minsky.canvas.item.table.getCell(1, 1)=="f11", "3rd insert"

for r in range(3):
  for c  in range(3):
     minsky.canvas.item.setCell(r, c, 'f'+str(r)+str(c))

# test column variables
assert minsky.canvas.item.table.getColumnVariables()()==['f01', 'f02']
# interior variables
assert minsky.canvas.item.table.getVariables()()==['f11', 'f12', 'f21', 'f22']

# test delete column
item.table.deleteCol(2)
assert minsky.canvas.item.table.getColumnVariables()()==['f02']
assert minsky.canvas.item.table.getVariables()()==['f12', 'f22']
assert minsky.canvas.item.table.getCell(1, 1)=="f12"
item.deleteRow(2)
assert minsky.canvas.item.table.getColumnVariables()()==["f02"]
assert minsky.canvas.item.table.getVariables()()==["f22"]
assert minsky.canvas.item.table.getCell(1, 1)=="f22"
assert minsky.canvas.item.table.rows()==2
# minsky inserted 2 extra blank colums for liabilities and equitites 
assert minsky.canvas.item.table.cols()==4,"1"

assert minsky.canvas.item.table.getVariables()()==["f22"]

# test rowsum, and stricter test of get variables
item.table.clear()
item.table.resize(4, 4)
assert minsky.canvas.item.table.rows()==4
assert minsky.canvas.item.table.cols()==4
# reset all columsn to "asset"
item.setCell(0, 1, 's1')
item.setCell(0, 2, 's2')
item.setCell(0, 3, 's3')
item.setCell(1, 0, "Initial Conditions")
item.setCell(1, 1, '10')
item.setCell(1, 3, '-10')
item.setCell(2, 1, 'a')
item.setCell(2, 2, 'b')
item.setCell(2, 3, '-a')
assert minsky.canvas.item.table.rowSum(1)=='0'
assert minsky.canvas.item.table.rowSum(2)=='b'
assert minsky.canvas.item.table.getVariables()()==['a', 'b']

minsky.canvas.deleteItem()
assert minsky.model.numItems()==0
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
