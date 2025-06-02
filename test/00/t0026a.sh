#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >cut-paste.py <<EOF
from pyminsky import minsky, findObject, findVariable
model=minsky.model
canvas=minsky.canvas

minsky.load('$here/examples/GoodwinLinear02.mky')
canvas.recentre()
assert model.numItems()==26
assert model.numWires()==27
# find "N" and use this work out a selection area
item=findVariable('N')
x1=item.x()-15
y1=item.y()-73
x2=item.x()+57
y2=item.y()+19
# weirdly, this command is required to get the selection to work below
canvas.recentre()

canvas.mouseDown(x1, y1)
canvas.mouseUp(x2, y2)
assert canvas.selection.numItems()==3
assert canvas.selection.numWires()==2
minsky.cut()
assert model.numItems()==23
assert model.numWires()==23
# For ticket 1098. Paste items at mouse position
minsky.paste(450, 106)

assert model.numItems()==26
assert model.numWires()==25

# find a wire with internal control points
wire=None
for i in range(len(model.wires)):
  if len(model.wires[i].coords())>4:
    wire=model.wires[i]
    break
assert wire is not None

wire.straighten()
assert len(wire.coords())==4

# some code that prints the values used in the next wiring op
for i in range(len(model.items)):
  item=model.items[i]
  if item.classType()=="Operation:divide":
    x0=item.portX(0)
    y0=item.portY(0)
  if item.classType()=="Variable:flow" and item.name()=="emprate":
    x1=item.portX(1)
    y1=item.portY(1)

# add another wire
canvas.mouseDown(x0, y0)
canvas.mouseUp(x1, y1)
assert model.numWires()==26

EOF

python3 cut-paste.py
if test $? -ne 0; then fail; fi

pass
