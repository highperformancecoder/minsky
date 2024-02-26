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
cat >input.tcl <<EOF
minsky.load $here/examples/exponentialGrowth.mky
minsky.numBackups 3
minsky.findObject "IntOp"
for {set i 1} {\$i<10} {incr i} {
  minsky.canvas.item.description "y\$i"
  minsky.save foo.mky
}
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

if [ `grep "<name>y" foo.mky*|wc -l` -ne 4 ]; then fail; fi

for i in 3 2 1; do
    if ! grep "<name>y$[9-i]" "foo.mky;$i" >/dev/null; then fail; fi
done

pass
