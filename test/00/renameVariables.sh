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
sys.path.insert(0,'$here')
from pyminsky import minsky
minsky.load('$here/examples/GoodwinLinear02.mky')

def findVariable(name):
    for i in range(len(minsky.model.items)):
        try:
          if minsky.model.items[i].name()==name:
             minsky.canvas.getItemAt(minsky.model.items[i].x(),minsky.model.items[i].y())
             return True
        except:
          pass

assert findVariable('L')
minsky.canvas.renameAllInstances('R')
assert not findVariable('L')
assert findVariable('R')
minsky.canvas.selectAllVariables()
assert len(minsky.canvas.selection.items)==2

# check findDefinition
assert findVariable('R')
minsky.canvas.findVariableDefinition()
assert minsky.canvas.itemIndicator().name()=="R"
# check that the selected variable has its input wired
assert minsky.canvas.itemIndicator().inputWired()
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
