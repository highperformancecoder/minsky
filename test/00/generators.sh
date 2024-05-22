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
import sys
sys.path.append('$here')
from pyminsky import minsky

minsky.canvas.addVariable('gen','parameter')

for op in ['eye', 'one', 'zero', 'iota']:
  minsky.canvas.itemFocus().init(f'{op}(3,3)')
  minsky.reset()
  minsky.canvas.itemFocus().exportAsCSV(f'{op}.csv',False)
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

for i in *.csv; do
    diff $i $here/test/generatorExemplars/$i
    if [ $? -ne 0 ]; then fail; fi
done
pass
