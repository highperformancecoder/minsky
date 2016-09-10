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
minsky.load \$argv(2)
minsky.reset
tcl_exit [expr ![minsky.checkEquationOrder]]
EOF

# I think this test is probably obsolete now, as the new equation structure guarantees correct order
pass

for i in $here/examples/*.mky; do
    echo "checking $i"
    $here/GUI/minsky input.tcl $i
    if test $? -ne 0; then 
        echo "for $i"
        fail
    fi
done

pass
