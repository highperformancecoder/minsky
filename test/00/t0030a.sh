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
cat >input.tcl <<EOF
source $here/test/assert.tcl
proc afterMinskyStarted {} {
  minsky.save no
  minsky.load $here/examples/1Free.mky
  recentreCanvas
  assert {[findObject GodleyIcon]}
  set item minsky.canvas.item
  doubleButton [\$item.x] [\$item.y]
  update
  # should open Godley window
  assert {[lsearch -glob [winfo children .] .godley*]>=0}

  # now double click on one of the flow variables
  findVariable IntLoan
  set var minsky.canvas.item
  doubleButton [\$var.x] [\$var.y]
  assert {[winfo viewable .wiring.editVar]}
  .wiring.editVar.buttonBar.ok invoke
  assert {![winfo exists .wiring.editVar]}

  assert {[findVariable Loan]}
  editItem
  assert {[winfo viewable .wiring.editVar]} {varclick}
  .wiring.editVar.buttonBar.ok invoke
  assert {![winfo exists .wiring.editVar]} {varclick}


  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
