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

# basic godley table tests

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.addGodley
set item minsky.canvas.itemFocus
minsky.getItemAt [\$item.x] [\$item.y]
\$item.table.clear
assert {[minsky.canvas.item.table.rows]==0} ""
assert {[minsky.canvas.item.table.cols]==0} ""

# check insertRow/insertCol
\$item.table.insertRow 0
\$item.table.insertCol 0
assert {[minsky.canvas.item.table.rows]==1} ""
assert {[minsky.canvas.item.table.cols]==1} ""
\$item.setCell 0 0 f11
assert {[minsky.canvas.item.table.getCell 0 0]=="f11"} ""
\$item.table.insertRow 0
\$item.table.insertCol 0
assert {[minsky.canvas.item.table.rows]==2} ""
assert {[minsky.canvas.item.table.cols]==2} ""
assert {[minsky.canvas.item.table.getCell 1 1]=="f11"} ""
\$item.table.insertRow 2
\$item.table.insertCol 2
assert {[minsky.canvas.item.table.rows]==3} ""
assert {[minsky.canvas.item.table.cols]==3} ""
assert {[minsky.canvas.item.table.getCell 1 1]=="f11"} ""
for {set r 0} {\$r<3} {incr r} {
  for {set c 0} {\$c<3} {incr c} {
     minsky.canvas.item.setCell \$r \$c f\$r\$c
  }
}
# test column variables
assert {"[minsky.canvas.item.table.getColumnVariables]"=="f01 f02"} ""
# interior variables
assert {"[minsky.canvas.item.table.getVariables]"=="f11 f12 f21 f22"} ""

# test delete column
\$item.table.deleteCol 2
assert {"[minsky.canvas.item.table.getColumnVariables]"=="f02"} ""
assert {"[minsky.canvas.item.table.getVariables]"=="f12 f22"} ""
assert {[minsky.canvas.item.table.getCell 1 1]=="f12"} ""
\$item.deleteRow 2
assert {"[minsky.canvas.item.table.getColumnVariables]"=="f02"} ""
assert {"[minsky.canvas.item.table.getVariables]"=="f22"} ""
assert {[minsky.canvas.item.table.getCell 1 1]=="f22"} ""
assert {[minsky.canvas.item.table.rows]==2} ""
# minsky inserted 2 extra blank colums for liabilities and equitites 
assert {[minsky.canvas.item.table.cols]==4} "1"

assert {"[minsky.canvas.item.table.getVariables]"=="f22"} ""

# test rowsum, and stricter test of get variables
\$item.table.clear
\$item.table.resize 4 4
assert {[minsky.canvas.item.table.rows]==4} ""
assert {[minsky.canvas.item.table.cols]==4} ""
# reset all columsn to "asset"
\$item.setCell 0 1 s1
\$item.setCell 0 2 s2
\$item.setCell 0 3 s3
\$item.setCell 1 0 "Initial Conditions"
\$item.setCell 1 1 10
\$item.setCell 1 3 -10
\$item.setCell 2 1 a
\$item.setCell 2 2 b
\$item.setCell 2 3 -a
assert {[minsky.canvas.item.table.rowSum 1]==0} ""
assert {[minsky.canvas.item.table.rowSum 2]=="b"} ""
assert {"[minsky.canvas.item.table.getVariables]"=="a b"} ""

minsky.canvas.deleteItem
assert {[minsky.model.numItems]==0} ""
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
