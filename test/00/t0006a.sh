#! /bin/sh

# test that a simple integral example works when both coupled and uncoupled

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

    minsky.load $here/examples/exponentialGrowth.mky
    
    getValue :y
    assert {[value.value]==1}

    set item "minsky.canvas.item"
    findObject VarConstant
    set a [\$item.value]
    assert {[minsky.canvas.item.value]!=0}

    # setting this flag prevents an extra reset occurring - used to
    # distinguish between static plot updates and simulation
    running 1 
    minsky.reset
    step
    step  
    getValue :y
    assert {[value.value]>0}
    if {\$a<0} {
      assert {[value.value]>0 && [value.value]<1}
    } else {
      assert {[value.value]>0 && [value.value]>1}
    }

    set v [value.value]

    findObject IntOp
    \$item.toggleCoupled

    minsky.reset
    step
    step
    getValue :y
    if {\$a<0} {
      assert {[value.value]>0 && [value.value]<1}
    } else {
      assert {[value.value]>0 && [value.value]>1}
    }
    assert "\[value.value\]==\$v"

    tcl_exit
}

EOF
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
