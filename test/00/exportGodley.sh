#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

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
