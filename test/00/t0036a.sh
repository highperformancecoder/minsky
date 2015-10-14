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
  updateCanvas
  select [expr [group.x]-0.5*[group.width]] [expr [group.y]-0.5*[group.height]] [expr [group.x]+0.5*[group.width]] [expr [group.y]+0.5*[group.height]] 
  newSystem
  group.get [paste]
  assert {[operations.size]==4}
  assert {[variables.size]==4}
  assert {[wires.size]==8}
  assert {[llength [group.groups]]==1}
  # retrieve inner group
  group.get [group.groups]
  assert {[llength [group.operations]]==4}
  assert {[llength [group.variables]]==4}
  assert {[llength [group.wires]]==8}
  resetEdited
  exit
}}
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
