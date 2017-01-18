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
assert {[items.size]==27} {}
assert {[wires.size]==27} {}
lasso 378 27
lassoEnd  450 106
cut
assert {[items.size]==24} {}
assert {[wires.size]==23} {}
set gid [paste]
insertNewGroup \$gid
event generate .wiring.canvas <Button-1>

buildMaps
assert {[items.size]==28} {}
assert {[wires.size]==25} {}

# find a wire with internal control points
for {set i 0} {\$i<[wires.size]} {incr i} {
  set w [lindex [wires.#keys] \$i]
  wire.get \$w
  if {[llength [wire.coords]]>4} break
}
decorateWire \$w
assert {[llength [wire.coords]]>4}
straightenWire \$w
assert {[llength [wire.coords]]==4}

# add another wire
set item [lindex [items.#keys] 0]
wires::startConnect \$item 405.5 170
wires::extendConnect \$item 400 170
wires::finishConnect \$item 183 173
assert {[wires.size]==26} {}

set w [lindex [wires.#keys] 0]
wire.get \$w 
decorateWire \$w
set handle [lindex [.wiring.canvas find withtag handles] 0]
insertCoords \$w \$handle 0 100 100
deleteHandle \$w \$handle 0

foreach it [items.#keys] {
  item.get \$it
  switch -glob [item.classType] { 
    "Variable*" {
      editItem \$it
      .wiring.editVar.buttonBar.ok invoke
     }
    "Operation*" {
      editItem \$it
      .wiring.editOperation.buttonBar.ok invoke
    }
  }
}

placeNewNote
event generate .wiring.canvas <Button-1>

tcl_exit
}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky cut-paste.tcl
if test $? -ne 0; then fail; fi

pass
