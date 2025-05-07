#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky, findObject

# Add an integration operation to the canvas
minsky.canvas.addOperation("integrate")

# Use findObject to locate the integration operation (IntOp)
integrationOp = findObject("IntOp")
assert integrationOp is not None, "Integration operation not found or not focused"

# Use findObject to locate the integral variable (Variable:integral)
integralVariable = findObject("Variable:integral")
assert integralVariable is not None, "Integral variable not found or not focused"
value_id=integralVariable.valueId()

# Ensure the value_id is valid
assert value_id is not None, "Integral variable does not have a valueId"

# Verify that the integral variable input is wired
assert minsky.inputWired(value_id), "Integral variable input is not wired"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
