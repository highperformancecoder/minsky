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
    
    value.get :y
    assert {[value.value]==1}

    var.get 33 # constant a
    set a [var.value]
    assert {[var.value]!=0}
    minsky.reset
    step
    step  
    value.get :y
    assert {[value.value]>0}
    if {\$a<0} {
      assert {[value.value]>0 && [value.value]<1}
    } else {
      assert {[value.value]>0 && [value.value]>1}
    }

    set v [value.value]

    minsky.reset
    integral.get 34 # integral
    integral.toggleCoupled

    step
    step
    value.get :y
    if {\$a<0} {
      assert {[value.value]>0 && [value.value]<1}
    } else {
      assert {[value.value]>0 && [value.value]>1}
    }
    assert "\[value.value\]==\$v"

    resetEdited
    exit
}

EOF
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
