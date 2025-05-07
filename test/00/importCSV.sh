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

minsky.canvas.addVariable('par','parameter')
csvDialog=minsky.variableValues[':par'].csvDialog
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
