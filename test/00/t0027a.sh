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
runstop
simulate
after 500 assert {[t]>0}
runstop
set tt [t]
after 500 assert {[t]>0}
assert "\$tt==[t]"
reset
assert {[t]==0}
newSystem
assert {[minsky.variables.size]==0}
resetEdited
exit
}}
EOF

cp $here/test/assert.tcl .
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
