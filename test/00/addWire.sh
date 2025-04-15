#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

#check addWire business rules
cat >input.py <<EOF
import sys
sys.path.insert(0, '$here')
from pyminsky import minsky

minsky.canvas.addOperation('exp')
minsky.canvas.itemFocus.dummyDraw()
x0=minsky.canvas.itemFocus.portX(0)
y0=minsky.canvas.itemFocus.portY(0)

minsky.canvas.addOperation('exp')
minsky.canvas.itemFocus.dummyDraw()
minsky.canvas.itemFocus.moveTo(100, 100)
x1=minsky.canvas.itemFocus.portX(1)
y1=minsky.canvas.itemFocus.portY(1)
# add a wire
minsky.canvas.mouseDown(x0, y0)
minsky.canvas.mouseUp(x1, y1)
assert len(minsky.model.wires)==1
minsky.canvas.mouseDown(x0, y0)
minsky.canvas.mouseUp(x1, y1)

assert len(minsky.model.wires)==1,"duplicate wire added!"
minsky.canvas.mouseDown(x0, y0)
minsky.canvas.mouseUp(x0, y0)
assert len(minsky.model.wires)==1,"self wire allowed!"
minsky.canvas.getWireAt(x0, y0)
minsky.canvas.deleteWire()
assert minsky.model.numWires()==0
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
