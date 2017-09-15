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

cat >cut-paste.tcl <<EOF
source assert.tcl
proc afterMinskyStarted {} {

minsky.load $here/examples/GoodwinLinear02.mky
recentreCanvas
assert {[model.numItems]==26} {}
assert {[model.numWires]==27} {}
canvas.mouseDown 378 14
canvas.mouseUp  450 106
assert {[canvas.selection.numItems]==3} {}
assert {[canvas.selection.numWires]==2} {}
cut
assert {[model.numItems]==23} {}
assert {[model.numWires]==23} {}
paste

event generate .wiring.canvas <Button-1>

assert {[model.numItems]==26} {}
assert {[model.numWires]==25} {}

# find a wire with internal control points
for {set i 0} {\$i<[model.wires.size]} {incr i} {
  model.wires.@elem [set i]
  if {[llength [minsky.model.wires(\$i).coords]]>4} {
    set wire minsky.model.wires(\$i)
    break
  }
}

\$wire.straighten
assert "[llength [[set wire].coords]]==4" {llength wire.coords==4}

# add another wire
canvas.mouseDown 357 25
canvas.mouseUp 480 157
assert {[model.numWires]==26} {}

tcl_exit
}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky cut-paste.tcl
if test $? -ne 0; then fail; fi

pass
