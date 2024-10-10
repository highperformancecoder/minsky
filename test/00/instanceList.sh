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
minsky.findVariable "Y"
set instanceList [minsky.listAllInstances]
assert "[llength [\$instanceList.names]]==2"
set offsx [minsky.model.x]
set offsy [minsky.model.y]
\$instanceList.gotoInstance 0
set offsx0 [minsky.model.x]
set offsy0 [minsky.model.y]
assert "\$offsx0!=\$offsx"
assert "\$offsy0!=\$offsy"


# offsets hard coded in VariableInstanceList
minsky.canvas.getItemAt [expr \$offsx-[minsky.model.x]+50] [expr \$offsy-[minsky.model.y]+50]
assert {[minsky.canvas.item.name]=="Y"}

\$instanceList.gotoInstance 1
assert "[minsky.model.x]!=\$offsx0"
assert "[minsky.model.y]!=\$offsy0"

minsky.canvas.getItemAt [expr \$offsx-[minsky.model.x]+50] [expr \$offsy-[minsky.model.y]+50]
assert {[minsky.canvas.item.name]=="Y"}

assert "[catch \$instanceList.gotoInstance 2]"
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass