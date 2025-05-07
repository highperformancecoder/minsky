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

cat >input.py <<EOF
from pyminsky import minsky
minsky.load('$here/examples/1Free.mky')
for i in range(len(minsky.model.items)):
    item=minsky.model.items[i]
    if item.classType()=='GodleyIcon':
       item.table.exportToLaTeX('1FreeGodley.tex')
       item.table.exportToCSV('1FreeGodley.csv')
       break

minsky.load('$here/test/testEq.mky')
for i in range(len(minsky.model.items)):
    item=minsky.model.items[i]
    if item.classType()=='GodleyIcon':
       item.table.exportToLaTeX('testEqGodley.tex')
       item.table.exportToCSV('testEqGodley.csv')
       break
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

for i in *.tex *.csv; do
    diff $i $here/test/$i
    if [ $? -ne 0 ]; then fail; fi
done

pass
