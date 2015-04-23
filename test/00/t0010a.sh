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
set ports0 [minsky.op.ports]
minsky.op.get \$op1
set ports1 [minsky.op.ports]
assert "[minsky.addWire [lindex \$ports0 0] [lindex \$ports1 1]]==-1" "wire needs coordinates"
set wireAdded [minsky.addWire [lindex \$ports0 0] [lindex \$ports1 1] {0 0 0 0}]
assert "\$wireAdded!=-1" ""
assert "[minsky.addWire [lindex \$ports0 0] [lindex \$ports1 1] {0 0 0 0}]==-1" "duplicate wire added!"
assert "[minsky.addWire [lindex \$ports0 0] [lindex \$ports0 1] {0 0 0 0}]==-1" "self wire allowed!"
assert "[minsky.addWire [lindex \$ports0 1] [lindex \$ports1 0] {0 0 0 0}]==-1" "input wired to output!"
minsky.wire.get \$wireAdded
assert "[minsky.wire.from]==[lindex \$ports0 0] && [minsky.wire.to]==[lindex \$ports1 1]" ""
minsky.deleteWire \$wireAdded
assert {[minsky.wires.size]==0} ""
tcl_exit
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
