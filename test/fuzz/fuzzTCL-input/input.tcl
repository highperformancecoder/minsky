source test/assert.tcl
proc afterMinskyStarted {} {
  minsky.load examples/1Free.mky
  findObject GodleyIcon
  set id [minsky.openGodley]
  openGodley $id
  $id.displayStyle sign
  $id.displayValues 1
  $id.renderCanvasToSVG 1FreeBase.svg
  $id.selectedCol 1
  $id.selectedRow 2
  $id.insertIdx 0
  $id.selectIdx 3
  $id.hoverRow 2
  $id.hoverCol 2
  $id.mouseMoveB1 [expr [lindex [$id.colLeftMargin] 2]+10] [expr 4*[$id.rowHeight]+[$id.topTableOffset]]
  $id.displayStyle DRCR
  $id.renderCanvasToSVG 1Free11Selected.svg
  $id.selectedCol 0
  $id.renderCanvasToSVG 1FreeSelectedRow.svg
  $id.selectedCol 1
  $id.selectedRow 0
  $id.mouseMoveB1 [expr [lindex [$id.colLeftMargin] 2]+10] [expr 3*[$id.rowHeight]+[$id.topTableOffset]]
  $id.renderCanvasToSVG 1FreeSelectedCol.svg
  

  minsky.load /home/rks/github/minsky2/examples/LoanableFunds.mky
  findObject GodleyIcon
  set id [minsky.openGodley]
  $id.displayStyle DRCR
  $id.displayValues 0
  $id.renderCanvasToSVG LoanableFundsBase.svg

  tcl_exit
}
