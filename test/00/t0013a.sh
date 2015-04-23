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
set godley [minsky.addGodleyTable 0 0]
minsky.godley.get \$godley
minsky.godley.table.clear
minsky.godley.table.resize 4 4
minsky.godley.table.setDEmode 1
minsky.godley.setCell 1 0 "Initial Conditions"
minsky.godley.setCell 1 1 10
minsky.godley.setCell 1 3 -10
minsky.godley.setCell 2 1 a
minsky.godley.setCell 2 2 b
minsky.godley.setCell 2 3 -a

# TODO - this test will need to be modified when ticket #58 is dealt with
minsky.godley.table.assetClass 1 asset
minsky.godley.table.assetClass 2 liability
minsky.godley.table.assetClass 3 liability
assert {[minsky.godley.table.rowSum 1]==0} ""
assert {[minsky.godley.table.rowSum 2]=="b"} ""
assert {"[minsky.godley.table.getVariables]"=="a b"} ""
tcl_exit
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
