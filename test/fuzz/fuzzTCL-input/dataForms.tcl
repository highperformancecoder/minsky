source /home/rks/github/minsky2/test/assert.tcl
proc tk_getOpenFile {args} {
  return "test.dat"
}

proc afterMinskyStarted {} {
  addOperation data
  findObject DataOp
  importData

  assert {[minsky.canvas.item.data.#keys]=="1 2 6"}

  initRandom
  .wiring.initRandom.xmax.entry delete 0 end
  .wiring.initRandom.xmax.entry insert 0 3
  .wiring.initRandom.numVals.entry delete 0 end
  .wiring.initRandom.numVals.entry insert 0 3
  .wiring.initRandom.buttonBar.ok invoke
  assert {[minsky.canvas.item.data.#keys]=="0 1 2"}

  tcl_exit
}
