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
EOF

cat >test.dat <<EOF
1 3
2 4
6 5
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
