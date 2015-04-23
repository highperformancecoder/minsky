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

# test that save file throws an error if it can't
cat >input.tcl <<EOF
minsky.save writeable.mky
if [catch {minsky.save readonly.mky}] {tcl_exit 0}
tcl_exit 1
EOF

touch readonly.mky
chmod a-w readonly.mky
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
