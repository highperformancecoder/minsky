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
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source assert.tcl
proc afterMinskyStarted {} {
uplevel #0 {
openNamedFile $here/examples/GoodwinLinear02.mky
pushFlags
runstop
assert {[running] == 1}
simulate
after 500 {runstop; set running 0}
vwait running
assert {[t]>0}
set tt [t]
reset
assert {[t]==0}
popFlags
newSystem
assert {[minsky.model.numItems]==0}
assert {[minsky.model.numWires]==0}
assert {[minsky.model.numGroups]==0}
tcl_exit
}}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
