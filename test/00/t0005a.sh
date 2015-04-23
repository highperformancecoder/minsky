#! /bin/sh

# test that self wiring of variables and operators is prevented

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

# arrange for the following code to executed after Minsky has started.
proc afterMinskyStarted {} {
    # remove Destroy binding as it interferes with assert
    bind . <Destroy> {}

    
    op.get [addOperation  exp]
    assert {[addWire [lindex [op.ports] 0] [lindex [op.ports] 1] {0 0 0 0}]==-1}
    assert {[wires.size]==0}
    
    var.get [newVariable foo flow]
    assert {[addWire [var.outPort] [var.inPort] {0 0 0 0}]==-1}
    assert {[wires.size]==0}

    resetEdited
    exit
}

EOF
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
