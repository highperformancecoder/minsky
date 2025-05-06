#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky, findObject, findVariable

minsky.timeUnit('s')
minsky.canvas.addGodley()
item=findObject('GodleyIcon')
item.table.resize(4, 4)
item.setCell(0, 1, 'a')
item.setCell(2, 1, 'b')
item.setCell(3, 1, 'c')
item.update()
findVariable('b').setUnits('b')
findVariable('c').setUnits('c')

# should throw incompatible units
thrown=False
try:
    minsky.dimensionalAnalysis()
except:
    thrown=True
assert thrown
findVariable('c').setUnits('b')

thrown=False
try:
    minsky.dimensionalAnalysis()
except:
    thrown=True
assert thrown

item=findVariable('a')
item.setUnits("b s")
assert item.unitsStr()=="b s"
minsky.dimensionalAnalysis()
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
