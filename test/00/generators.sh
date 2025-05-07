#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.canvas.addVariable('gen','parameter')

for op in ['eye', 'one', 'zero', 'iota']:
  minsky.canvas.itemFocus.init(f'{op}(3,3)')
  minsky.reset()
  minsky.canvas.itemFocus.exportAsCSV(f'{op}.csv',False)
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

for i in *.csv; do
    diff $i $here/test/generatorExemplars/$i
    if [ $? -ne 0 ]; then fail; fi
done
pass
