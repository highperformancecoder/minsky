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
minsky.timeUnit s
minsky.addGodley
minsky.findObject GodleyIcon
minsky.canvas.item.table.resize 4 4
minsky.canvas.item.setCell 0 1 a
minsky.canvas.item.setCell 2 1 b
minsky.canvas.item.setCell 3 1 c
minsky.canvas.item.update
minsky.findVariable b
minsky.canvas.item.setUnits b
minsky.findVariable c
minsky.canvas.item.setUnits c
# should throw incompatible units
assert {[catch minsky.dimensionalAnalysis]}
minsky.findVariable c
minsky.canvas.item.setUnits b
assert {[catch minsky.dimensionalAnalysis]}
minsky.findVariable a
minsky.canvas.item.setUnits "b s"
assert {[minsky.canvas.item.unitsStr]=="b s"}
minsky.dimensionalAnalysis

tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
