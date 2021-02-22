#! /bin/sh

here=`pwd`
if test $? -ne 0; then exit 2; fi
tmp=/tmp/$$
mkdir $tmp
if test $? -ne 0; then exit 2; fi
cd $tmp
if test $? -ne 0; then exit 2; fi

fail()
{
    echo "FAILED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 1
}

pass()
{
    echo "PASSED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 0
}

trap "fail" 1 2 3 15

#check addWire business rules
cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.addOperation exp
minsky.canvas.itemFocus.dummyDraw
set x0 [minsky.canvas.itemFocus.portX 0]
set y0 [minsky.canvas.itemFocus.portY 0]

minsky.addOperation exp
minsky.canvas.itemFocus.dummyDraw
minsky.canvas.itemFocus.moveTo 100 100
set x1 [minsky.canvas.itemFocus.portX 1]
set y1 [minsky.canvas.itemFocus.portY 1]
# add a wire
minsky.canvas.mouseDown \$x0 \$y0
minsky.canvas.mouseUp \$x1 \$y1
assert {[minsky.model.wires.size]==1} ""
minsky.canvas.mouseDown \$x0 \$y0
minsky.canvas.mouseUp \$x1 \$y1

assert {[minsky.model.wires.size]==1} "duplicate wire added!"
minsky.canvas.mouseDown \$x0 \$y0
minsky.canvas.mouseUp \$x0 \$y0
assert {[minsky.model.wires.size]==1} "self wire allowed!"
minsky.canvas.getWireAt \$x0 \$y0
minsky.canvas.deleteWire
assert {[minsky.model.numWires]==0} ""
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
