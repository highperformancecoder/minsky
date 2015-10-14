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
proc afterMinskyStarted {} {uplevel #0 {
 minsky.load $here/examples/GoodwinLinear02.mky
 recentreCanvas
 updateCanvas

 saveGroupAsFile [group.id] yyy.mky
 set eventRecord [open $here/test/groupButton1Recording.tcl r]
  while {[gets \$eventRecord cmd]>=0} {
    eval \$cmd
    update
    after 10
  }
 minsky.copy
 saveSelectionAsFile xxx.mky
 group.get [paste]
 assert {[llength [group.operations]]==2}
 resetEdited
 minsky.load xxx.mky
 assert {[operations.size]==2}
 assert {[variables.size]==0}
 assert {[wires.size]==1}

 # now test group save to file
 minsky.load yyy.mky
 assert {[operations.size]==4}
 assert {[variables.size]==4}
 assert {[wires.size]==8}

 exit
}}

EOF

#disabled!
pass

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi


pass
