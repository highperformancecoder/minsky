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

# check that the clearAll methods really do clear every
cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.load \$argv(2)
minsky.clearAll
assert {[minsky.wires.size]==0} \$argv(2)
assert {[minsky.items.size]==0} \$argv(2)
# should be just constant:zero and constant:one
assert {[minsky.variableValues.size]==2} \$argv(2)
assert {[minsky.model.items.size]==0} \$argv(2)
assert {[minsky.model.wires.size]==0} \$argv(2)
assert {[minsky.model.groups.size]==0} \$argv(2)
tcl_exit
EOF

echo $here

for i in $here/examples/*.mky; do
    $here/gui-tk/minsky input.tcl $i
    if test $? -ne 0; then fail; fi
    done

pass
