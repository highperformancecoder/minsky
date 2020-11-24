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
proc tk_messageBox args {return -code error "$args"}
minsky.load $here/examples/GoodwinLinear02.mky
proc afterMinskyStarted {} {
  assert {[findVariable L]}
  renameVariableInstances
  assert {[winfo ismapped .renameDialog]}
  .renameDialog.entry delete 0 end
  .renameDialog.entry insert 0 R
  .renameDialog.buttonBar.ok invoke
  assert {![findVariable L]}
  assert {[findVariable R]}
  canvas.selectAllVariables
  assert {[canvas.selection.items.size]==2}

  # check findDefinition
  findDefinition
  assert {[minsky.canvas.item.name]=="R"}
  # check that the selected variable has its input wired
  minsky.canvas.item.ports.@elem 1
  assert {[minsky.canvas.item.ports(1).numWires]==1}
  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
