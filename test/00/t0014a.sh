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

# check models reproduced logged data when run
cd $here/examples
for i in *.mky; do
    # math-examples is not a runnable example
    if [ $i = "math-examples.mky" ]; then continue; fi
    $here/GUI/minsky ../test/compareFileLog.tcl $i ../test/exampleLogs/$i.log
    if test $? -ne 0; then 
        echo "for $i"
        fail
    fi
done

pass
