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

# exercise item creation 
cat >input.tcl <<EOF
source assert.tcl
proc afterMinskyStarted {} {
uplevel #0 {

addVariable
set varInput(Name) "foo"
.wiring.initVar.buttonBar.ok invoke
event generate .wiring.canvas <Button-1> -x 100 -y 100
assert {[model.items.size]==1}

addConstant
.wiring.initVar.buttonBar.ok invoke
event generate .wiring.canvas <Button-1> -x 100 -y 100
assert {[model.items.size]==2}

addOperationKey add
assert {[model.items.size]==3}

textInput a
assert {[winfo exists .textInput]}
.textInput.buttonBar.ok invoke
assert {[winfo exists .wiring.editVar]}
.wiring.editVar.buttonBar.ok invoke
assert {[model.items.size]==4}

textInput f
assert {[winfo exists .textInput]}
.textInput.entry insert 1 "=0.2"
.textInput.buttonBar.ok invoke
assert {![winfo exists .wiring.editVar]}
assert {[model.items.size]==5}
assert {[findVariable f]} {}
assert {[minsky.canvas.item.init]=="0.2"}

textInput %
.textInput.entry insert 1 "b"
.textInput.buttonBar.ok invoke
assert {[model.items.size]==6}
assert {[findObject Item]} {}
# TODO reenable in python version
#assert {[minsky.canvas.item.detailedText]=="b"}

textInput s
.textInput.entry insert 1 "in"
.textInput.buttonBar.ok invoke
assert {[model.items.size]==7}
assert {[findObject "Operation:sin"]}

addGodley
event generate .wiring.canvas <Button-1>
assert {[model.items.size]==8}

addPlot
event generate .wiring.canvas <Button-1>
assert {[model.items.size]==9}


tcl_exit
}}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
