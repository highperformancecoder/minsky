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

# check that old schmea can still be read correctly
cd $here/test/oldSchema/schema0
for i in *.mky; do
    # this example has a constant and variable of the same name, so will fail this test
    echo $i
    if [ $i = 4MonetaryMinskyModelLessUnstableStart.mky ]; then continue; fi
    $here/GUI/minsky $here/test/rewriteMky.tcl $i $tmp/tmp.mky
    if test $? -ne 0; then fail; fi
    $here/GUI/minsky $here/test/rewriteMky.tcl $tmp/tmp.mky $tmp/tmp1.mky
    if test $? -ne 0; then fail; fi
# need to do it again, as Minsky will usually readjust the position of
# the canvas items
    $here/GUI/minsky $here/test/rewriteMky.tcl $tmp/tmp1.mky $tmp/tmp2.mky
    if test $? -ne 0; then fail; fi
    $here/GUI/minsky $here/test/rewriteMky.tcl $tmp/tmp2.mky $tmp/tmp3.mky
    if test $? -ne 0; then fail; fi
    $here/test/cmpFp  $tmp/tmp2.mky $tmp/tmp3.mky
    if test $? -ne 0; then 
        echo "old schema file $i failed to convert"
        fail
    fi
done

pass
