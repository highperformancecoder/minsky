#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# needs to be a jest test!
cat >input.tcl <<EOF
source $here/test/assert.tcl
proc afterMinskyStarted {} {
  canvasContext 0 0 0 0
  assert {[winfo ismapped .wiring.context]}
  .wiring.context unpost

  assert {![winfo ismapped .wiring.context]}
  wireContextMenu 0 0
  assert {[winfo ismapped .wiring.context]}
   .wiring.context unpost

  # need to check context on all types of items
  minsky.addVariable foo flow
  assert {[findObject Variable:flow]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "variable"
  .wiring.context unpost
  assert {![winfo ismapped .wiring.context]}

  minsky.addOperation integrate
  assert {[findObject IntOp]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "operation"
  .wiring.context unpost

  minsky.addOperation data
  assert {[findObject DataOp]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "data operation"
  .wiring.context unpost



  minsky.addPlot
  assert {[findObject PlotWidget]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "plot"
  .wiring.context unpost

  minsky.addGodley
  assert {[findObject GodleyIcon]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "godley"
  .wiring.context unpost

  minsky.addGroup
  assert {[findObject Group]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "group"
  .wiring.context unpost
  
  minsky.addNote "hello"
  assert {[findObject Item]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "note"
  .wiring.context unpost

  minsky.addSwitch
  assert {[findObject SwitchIcon]}
  contextMenu 0 0 0 0
  assert {[winfo ismapped .wiring.context]} "switch"
  .wiring.context unpost
  

  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
