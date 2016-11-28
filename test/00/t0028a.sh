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
event generate .wiring.canvas <Button-1>
assert {[items.size]==1}

addConstant
.wiring.initVar.buttonBar.ok invoke
event generate .wiring.canvas <Button-1>
assert {[items.size]==2}

addOperationKey add
assert {[items.size]==3}

textInput a
textInput \r
assert {[items.size]==4}
textInput %
textInput a
deleteKey
textInput b
textInput \r
assert {[items.size]==5}
assert {[item.detailedText]=="b"}

addNewGodleyItem
event generate .wiring.canvas <Button-1>
assert {[items.size]==6}

newPlot
event generate .wiring.canvas <Button-1>
assert {[items.size]==7}


tcl_exit
}}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
