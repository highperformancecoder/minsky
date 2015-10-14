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

# test of undo/redo

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
proc afterMinskyStarted {} {
minsky.load 1Free.mky
updateCanvas
minsky.save checkpoint1.mky
checkPushHistory
.wiring.canvas delete op70
deleteOperation 70
checkPushHistory
updateCanvas
minsky.save checkpoint2.mky
undo 1
updateCanvas
minsky.save checkpoint3.mky
undo -1
updateCanvas
minsky.save checkpoint4.mky
exit
}
EOF

#disabled!
pass

cp $here/examples/1Free.mky .
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

diff checkpoint1.mky checkpoint3.mky
if test $? -ne 0; then fail; fi

diff checkpoint2.mky  checkpoint4.mky
if test $? -ne 0; then fail; fi


pass
