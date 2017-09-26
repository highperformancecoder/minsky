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

cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.load $here/examples/GoodwinLinear02.mky
# find the group item
minsky.model.groups.@elem 0
minsky.getItemAt [minsky.model.groups(0).x] [minsky.model.groups(0).y]
assert {[minsky.canvas.item.classType]=="Group"}
minsky.openGroupInCanvas 
assert {[minsky.model.groups(0).numItems]==[minsky.canvas.model.numItems]}
assert {[minsky.model.groups(0).numWires]==[minsky.canvas.model.numWires]}
minsky.openModelInCanvas 
assert {[minsky.model.numItems]==[minsky.canvas.model.numItems]}
assert {[minsky.model.numWires]==[minsky.canvas.model.numWires]}
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
