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

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
# test copy/pasting of groups
cat >input.tcl <<EOF
source $here/test/assert.tcl
proc afterMinskyStarted {} {uplevel #0 {
  minsky.load $here/examples/GoodwinLinear02.mky
#  updateCanvas
  model.groups.@elem 0
  canvas.select [expr [minsky.model.groups(0).x]-0.55*[minsky.model.groups(0).width]] [expr [minsky.model.groups(0).y]-0.55*[minsky.model.groups(0).height]] [expr [minsky.model.groups(0).x]+0.55*[minsky.model.groups(0).width]] [expr [minsky.model.groups(0).y]+0.55*[minsky.model.groups(0).height]] 
  newSystem
  paste
  assert {[model.numGroups]==2}
  assert {[model.numItems]==8}
  assert {[model.numWires]==8}
  # retrieve inner group
  model.groups.@elem 0
  minsky.model.groups(0).groups.@elem 0

  assert {[minsky.model.groups(0).groups(0).items.size]==8}
  assert {[minsky.model.groups(0).groups(0).wires.size]==8}
  tcl_exit
}}
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
