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
sys.path.insert(0, '$here')
from pyminsky import minsky, findObject

# Add an integration operation to the canvas
minsky.canvas.addOperation("integrate")

# Use findObject to locate the integration operation (IntOp)
integrationOp = findObject("IntOp")
assert integrationOp is not None, "Integration operation not found or not focused"

# Use findObject to locate the integral variable (Variable:integral)
integralVariable = findObject("Variable:integral")
assert integralVariable is not None, "Integral variable not found or not focused"

# Match the integral variable's position with items in the model to retrieve valueId
value_id = None
for i in range(len(minsky.model.items)):
    item = minsky.model.items[i]
    if item.x() == integralVariable.x() and item.y() == integralVariable.y():
        value_id = item.valueId()
        break

# Ensure the value_id is valid
assert value_id is not None, "Integral variable does not have a valueId"

# Verify that the integral variable input is wired
assert minsky.inputWired(value_id), "Integral variable input is not wired"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
