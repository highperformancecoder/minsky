#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
# check if rectangle A is contained in rectangle B
def contained(A, B):
  return A.right()>=B.right() and A.left()<=B.left() and A.bottom()>=B.bottom() and A.top()<=B.top()

from pyminsky import minsky, findObject

minsky.load('testGroup.mky')
group=findObject('Group')
# initially, no internal items should be visible
for v in range(len(group.items)):
    var=group.items[v]
    assert not var.visible(), "var"+str(v)

for w in range(len(group.wires)):
    wire=group.wires[w]
    assert not wire.visible(), "wire"+str(w)

# zoom into displayZoom - everything should now be visible
minsky.canvas.zoomToDisplay()
group.updateBoundingBox()
for v in range(len(group.items)):
    var=group.items[v]
    var.updateBoundingBox()
    if not var.ioVar():
      assert var.visible(), "var"+str(v)
      # icon should be within content bounds
      assert contained(group.bb, var.bb), "var"+str(v)
    else:
      assert not var.visible(), "edge var"+str(v)

for w in range(len(group.wires)):
    wire=group.wires[w]
    assert wire.visible(), "wire\$w"

# zoom back to original
minsky.canvas.zoom(1/minsky.canvas.zoomFactor(),0,0)
for v in range(len(group.items)):
    var=group.items[v]
    assert not var.visible(), "var"+str(v)

for w in range(len(group.wires)):
    wire=group.wires[w]
    assert not wire.visible(), "wire"+str(w)
EOF

cp $here/test/testGroup.mky .
python3 input.py
if test $? -ne 0; then fail; fi

pass
