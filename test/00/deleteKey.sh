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
minsky.load $here/examples/GoodwinLinear02.mky
minsky.canvas.recentre
proc afterMinskyStarted {} {
  puts "minsky started"
  set numItems [model.numItems]
  set numWires [model.numWires]
  focus .wiring.canvas

#  set w [minsky.model.wires.@elem 1]
#  set x {[\$w.coords.@elem 0]}
#  set y {[\$w.coords.@elem 1]}
#  deleteKey \$x \$y
  deleteKey 415 290
  assert "[model.numWires]==[expr \$numWires-1]" {test wire deletion}
  findVariable emprate
  deleteKey [minsky.canvas.item.x] [minsky.canvas.item.y]
  assert "[model.numItems]==[expr \$numItems-1]" {test emprate var deletion}
  # should also be two more wires deleted
  assert "[model.numWires]==[expr \$numWires-3]" {test attached wires deletion}

  findVariable Investment
  canvas.selectAllVariables
  deleteKey 0 0
  assert "[model.numItems]==[expr \$numItems-2]" {test selection deletion}

  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
