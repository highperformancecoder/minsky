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
source $here/test/assert.tcl
proc afterMinskyStarted {} {	
  minsky.load $here/test/testEq.mky	
  minsky.canvas.recentre  
  .tabs select 1	
  assert {[lindex [.tabs tabs] [.tabs index current]]==".equations"}
  panCanvas 100 100
  minsky.equationDisplay.redraw 0 0 600 800
  assert {[minsky.equationDisplay.offsx]==100}
  assert {[minsky.equationDisplay.offsy]==100}
  exit
}
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
