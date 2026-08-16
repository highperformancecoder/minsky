#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# test copy/pasting of groups
cat >input.py <<EOF
from pyminsky import minsky, findObject, findVariable
minsky.load('$here/examples/GoodwinLinear02.mky')
item=findObject('Group')
canvas=minsky.canvas
model=minsky.model
canvas.mouseDown(item.left()-10, item.bottom()+10)
canvas.mouseUp(item.left()+10, item.bottom()-10) 
assert len(canvas.selection.groups)==1
minsky.clearAllMaps()
minsky.paste()
# For ticket 1080. There is no longer an outer group when existing groups or items are pasted between canvasses or in the same canvas
assert model.numGroups()==1
assert model.numItems()==8
assert model.numWires()==8
EOF

python3 input.py
if test $? -ne 0; then fail; fi

pass
