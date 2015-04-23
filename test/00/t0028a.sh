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
.wiring.initVar.buttonBar.ok invoke
event generate .wiring.canvas <Button-1>
assert {[variables.size]==1}

addConstant
.wiring.initVar.buttonBar.ok invoke
event generate .wiring.canvas <Button-1>
assert {[variables.size]==2}

addOperationKey add
assert {[operations.size]==1}

textInput a
textInput \r
assert {[variables.size]==3}
textInput %
textInput a
deleteKey
textInput b
textInput \r
assert {[notes.size]==1}
assert {[note.detailedText]=="b"}

addNewGodleyItem [addGodleyTable 10 10]
event generate .wiring.canvas <Button-1>
assert {[godleyItems.size]==1}

newPlot
event generate .wiring.canvas <Button-1>
assert {[plots.size]==1}


resetEdited
exit
}}
EOF

cp $here/test/assert.tcl .
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
