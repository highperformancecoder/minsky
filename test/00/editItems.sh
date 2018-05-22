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
proc afterMinskyStarted {} {
  # need to check context on all types of items
  minsky.addVariable foo flow
  assert {[findObject Variable:flow]}
  editItem
  assert {[winfo ismapped .wiring.editVar]} "variable"
  destroy .wiring.editVar
  assert {![winfo ismapped .wiring.context]}

  minsky.addOperation integrate
  assert {[findObject IntOp]}
  editItem
  assert {[winfo ismapped .wiring.editConstant]} "integral"
  destroy .wiring.editConstant

  minsky.addOperation data
  assert {[findObject DataOp]}
  editItem
  assert {[winfo ismapped .wiring.editConstant]} "data operation"
  destroy .wiring.editConstant

  minsky.addOperation exp
  assert {[findObject Operation:exp]}
  editItem
  assert {[winfo ismapped .wiring.editOperation]} "data operation"
  destroy .wiring.editOperation

  minsky.addPlot
  assert {[findObject PlotWidget]}
  assert {[lsearch -glob [winfo children .] .plot*]==-1} "plot"
  editItem
  assert {[lsearch -glob [winfo children .] .plot*]>=0} "plot"

  minsky.addGodley
  assert {[findObject GodleyIcon]}
  assert {[lsearch -glob [winfo children .] .godley*]==-1} "godley"
  editItem
  assert {[lsearch -glob [winfo children .] .godley*]>=0} "godley"

  minsky.addGroup
  assert {[findObject Group]}
  editItem
  assert {[winfo ismapped .wiring.editGroup]} "group"
  destroy .wiring.editGroup

  minsky.addNote "hello"
  assert {[findObject Item]}
  editItem
  assert {[winfo ismapped .wiring.note]} "note"
  destroy .wiring.note
  grab release .wiring.note

  minsky.addSwitch
  editItem
# should be do nothing ATM  

  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
