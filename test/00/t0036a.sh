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
  openNamedFile $here/examples/GoodwinLinear02.mky
  update
  assert {[findObject Group]}
  set item minsky.canvas.item
  canvas.mouseDown [expr [\$item.left]-10] [expr [\$item.bottom]+10]
  canvas.mouseUp [expr [\$item.left]+10] [expr [\$item.bottom]-10] 
  assert {[canvas.selection.groups.size]==1}
  newSystem
  paste
  # For ticket 1080. There is no longer an outer group when existing groups or items are pasted between canvasses or in the same canvas
  assert {[model.numGroups]==1}
  assert {[model.numItems]==8}
  assert {[model.numWires]==8}
  file delete [autoBackupName]
  tcl_exit
}}
EOF

rm $here/examples/GoodwinLinear02.mky#
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
