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

# Load the model and centre the canvas
minsky.load('$here/examples/GoodwinLinear02.mky')
minsky.canvas.recentre()

# Store initial counts of items and wires
numItems = minsky.model.numItems()
numWires = minsky.model.numWires()
print(f"Initial number of items: {numItems}, Initial number of wires: {numWires}")

# Function to delete an item or wire at a given position
def deleteKey(x, y):
    if minsky.canvas.getWireAt(x, y):
        minsky.canvas.deleteWire()
        print(f"Wire deleted at ({x}, {y})")
    elif minsky.canvas.getItemAt(x, y):
        item = minsky.canvas.item
        if item is not None:
            print(f"Deleting item at ({x}, {y}), ID: {item.id()}, Type: {item.classType()}")
            minsky.canvas.deleteItem()
        else:
            print(f"Item found but not focused at ({x}, {y}), manually deleting.")
            minsky.canvas.deleteItem()
    else:
        print(f"No item found at ({x}, {y}).")

# Step 1: Delete the wire at coordinates (450, 50)
deleteKey(450, 50)
assert minsky.model.numWires() == (numWires - 1), "Test wire deletion failed."

# Step 2: Delete the variable 'emprate'
try:
    variable_emprate = findVariable("emprate")
    assert variable_emprate is not None, "Variable 'emprate' not found in the model."
    deleteKey(variable_emprate.x(), variable_emprate.y())
    assert minsky.model.numItems() == (numItems - 1), "Test 'emprate' variable deletion failed."
    # Two wires should also be deleted after deleting the variable
    assert minsky.model.numWires() == (numWires - 3), "Test attached wires deletion failed."
except Exception as e:
    raise AssertionError(f"Error deleting variable 'emprate': {e}")

# Step 3: Delete the variable 'Investment'
try:
    variable_investment = findVariable("Investment")
    assert variable_investment is not None, "Variable 'Investment' not found in the model."
    minsky.canvas.selectAllVariables()
    deleteKey(variable_investment.x(), variable_investment.y())
    assert minsky.model.numItems() == (numItems - 2), "Test selection deletion failed."
except Exception as e:
    raise ValueError(f"Error deleting variable 'Investment': {e}")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
