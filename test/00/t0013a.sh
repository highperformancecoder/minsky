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

# double entry bookkeeping tests

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.addGodley
set item minsky.canvas.itemFocus
minsky.getItemAt [\$item.x] [\$item.y]
\$item.table.clear
\$item.table.resize 4 4
\$item.table.setDEmode 1
\$item.setCell 1 0 "Initial Conditions"
\$item.setCell 1 1 10
\$item.setCell 1 3 10
\$item.setCell 2 1 a
\$item.setCell 2 2 b
\$item.setCell 2 3 a

\$item.table.assetClass 1 asset
\$item.table.assetClass 2 liability
\$item.table.assetClass 3 liability
assert {[minsky.canvas.item.table.rowSum 1]==0} ""
assert {[minsky.canvas.item.table.rowSum 2]=="-b"} ""
assert {"[minsky.canvas.item.table.getVariables]"=="a b"} ""
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
