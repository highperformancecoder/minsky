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
assert {[variables.size]==15} {}
assert {[operations.size]==10} {}
assert {[wires.size]==27} {}
lasso 378 27
lassoEnd  450 106
cut
assert {[variables.size]==13} {}
assert {[operations.size]==9} {}
assert {[wires.size]==23} {}
set gid [paste]
insertNewGroup \$gid
event generate .wiring.canvas <Button-1>

assert {[variables.size]==15} {}
assert {[operations.size]==10} {}
assert {[wires.size]==25} {}

wire.get 81
decorateWire 81
assert {[llength [wire.coords]]>4}
straightenWire 81
assert {[llength [wire.coords]]==4}

# add another wire
wires::startConnect 16 xx 405.5 170
wires::extendConnect 16 xx 400 170
wires::finishConnect xx 183 173
assert {[wires.size]==26} {}

wire.get [lindex [wires.#keys] 0]
decorateWire [wire.id]
set handle [lindex [.wiring.canvas find withtag handles] 0]
insertCoords [wire.id] \$handle 0 100 100
deleteHandle [wire.id] \$handle 0

editItem 105 var105
.wiring.editVar.buttonBar.ok invoke
editItem 123 op123
.wiring.editOperation.buttonBar.ok invoke

placeNewNote
event generate .wiring.canvas <Button-1>

resetEdited
exit
}
EOF

cp $here/test/assert.tcl .
$here/GUI/minsky cut-paste.tcl
if test $? -ne 0; then fail; fi

pass
