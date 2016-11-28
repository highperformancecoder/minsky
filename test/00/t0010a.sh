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
set op0 [minsky.addOperation exp]
set op1 [minsky.addOperation exp]
minsky.op.get \$op0
set x0 [minsky.op.x]
set y0 [minsky.op.y]
minsky.op.get \$op1
minsky.op.moveTo 100 100
set x1 [minsky.op.x]
set y1 [minsky.op.y]
set wireAdded [minsky.addWire \$op0 \$x0 \$y0 \$x1 \$y1]
assert {[minsky.wires.size]==1} ""
assert "\$wireAdded!=-1" ""
assert "[minsky.addWire \$op0 \$x0 \$y0 \$x1 \$y1 {0 0 0 0}]==-1" "duplicate wire added!"
assert "[minsky.addWire \$op0 \$x0 \$y0 \$x0 \$y0 {0 0 0 0}]==-1" "self wire allowed!"
minsky.wire.get \$wireAdded
#assert "[minsky.wire.from]==[lindex \$ports0 0] && [minsky.wire.to]==[lindex \$ports1 1]" ""
minsky.deleteWire \$wireAdded
assert {[minsky.wires.size]==0} ""
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
