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
minsky.load $here/test/testEq.mky
.tabs select 1
minsky.equationDisplay.requestRedraw
assert {[lindex [.tabs tabs] [.tabs index current]]==".equations"}
#event generate .equations.canvas  <B1-Motion> -x 100 -y 100 -warp 1
minsky.equationDisplay.requestRedraw
puts [minsky.equationDisplay.width]
puts [minsky.equationDisplay.height]
set [minsky.equationDisplay.offsx] 100
set [minsky.equationDisplay.offsy] 100
assert {[minsky.equationDisplay.width]>0}
assert {[minsky.equationDisplay.height]>0}
exit
}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
