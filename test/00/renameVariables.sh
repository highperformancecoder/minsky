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
from pyminsky import minsky, findVariable

# Load the model
minsky.load('$here/examples/GoodwinLinear02.mky')

try:
    # Ensure the variable 'L' exists and get a reference to it
    variable_L = findVariable("L")
    assert variable_L is not None, "Variable 'L' not found in the model."
    print(f"Found variable 'L': {variable_L.name()} at ({variable_L.x()}, {variable_L.y()})")

    # Rename all instances of 'L' to 'R'
    minsky.canvas.renameAllInstances('R')

    # Ensure 'L' no longer exists
    variable_L = findVariable("L")
    assert variable_L is None, "Variable 'L' still exists after renaming."

    # Ensure 'R' now exists
    variable_R = findVariable("R")
    assert variable_R is not None, "Variable 'R' not found after renaming."
    print(f"Renamed variable: {variable_R.name()} at ({variable_R.x()}, {variable_R.y()})")

    # Select all variables and verify selection
    minsky.canvas.selectAllVariables()
    assert len(minsky.canvas.selection.items) == 2, "Selection count does not match."

    # Check findDefinition and wiring
    variable_R = findVariable("R")
    assert variable_R is not None, "Variable 'R' not found for findDefinition."
    minsky.canvas.findVariableDefinition()
    assert minsky.canvas.itemIndicator.name() == "R", "Selected variable for findDefinition is incorrect."
    assert minsky.canvas.itemIndicator.inputWired(), "Selected variable input is not wired."

except Exception as e:
    print(f"Test failed with exception: {e}")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
