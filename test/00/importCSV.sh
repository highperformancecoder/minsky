#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.canvas.addVariable('par','parameter')
csvDialog=minsky.variableValues[':par']
csvDialog.url('$here/gui-js/examples/data/PatentsByCountry1980-2011.csv')
csvDialog.loadFile()
assert(len(csvDialog.parseLines()())==csvDialog.numInitialLines())
# TODO - parseLines fails to report all lines in Python
csvDialog.guessSpecAndLoadFile()
assert(csvDialog.spec.nRowAxes()==1)
assert(csvDialog.spec.nColAxes()==2)
assert(csvDialog.spec.headerRow()==0)
assert(csvDialog.spec.numCols()==34)
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
