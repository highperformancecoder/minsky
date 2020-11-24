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
minsky.load $here/examples/1Free.mky
minsky.findObject GodleyIcon
minsky.canvas.item.table.exportToLaTeX 1FreeGodley.tex
minsky.canvas.item.table.exportToCSV 1FreeGodley.csv

minsky.load $here/test/testEq.mky
minsky.findObject GodleyIcon
minsky.canvas.item.table.exportToLaTeX testEqGodley.tex
minsky.canvas.item.table.exportToCSV testEqGodley.csv


tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

for i in *.tex *.csv; do
    diff $i $here/test/$i
    if [ $? -ne 0 ]; then fail; fi
done

pass
