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
from pyminsky import minsky

# Load the model and centre the canvas
minsky.load('$here/examples/GoodwinLinear02.mky')
minsky.canvas.recentre()

# Store initial counts of items and wires
numItems = minsky.model.numItems()
numWires = minsky.model.numWires()
print(f"Initial number of items: {numItems}, Initial number of wires: {numWires}")

# Function to find a variable by name
def findVariable(name):
    last_error = None
    for i in range(len(minsky.model.items)):
        try:
            if minsky.model.items[i].name() == name:
                x = minsky.model.items[i].x()
                y = minsky.model.items[i].y()
                print(f"Found variable '{name}' at coordinates ({x}, {y})")
                # Get the item at coordinates
                if minsky.canvas.getItemAt(x, y):
                    return (x, y)
                else:
                    print(f"Failed to focus item at ({x}, {y}). Trying manual fallback.")
                    return (x, y)  # Return coordinates anyway for manual handling
        except Exception as e:
            last_error = e  # Capture the error in the except block

    # Handle the error if it occurred
    if last_error:
        print(f"Error while finding variable '{name}' at last known coordinates: {last_error}")
    return None

# Function to delete an item or wire at a given position
def deleteKey(x, y):
    # Check for a wire or item at the coordinates and delete it
    if minsky.canvas.getWireAt(x, y):
        minsky.canvas.deleteWire()
        print(f"Wire deleted at ({x}, {y})")
    elif minsky.canvas.getItemAt(x, y):
        item = minsky.canvas.item()  # Try to focus on the item
        if item is not None:
            print(f"Deleting item at ({x}, {y}), ID: {item.id()}, Type: {item.classType()}")
            minsky.canvas.deleteItem()
        else:
            print(f"Item found but not focused at ({x}, {y}), manually deleting.")
            minsky.canvas.deleteItem()  # Manually delete if focus fails
    else:
        print(f"No item found at ({x}, {y}).")

# Step 1: Delete the wire at coordinates (415, 290)
deleteKey(415, 290)
assert minsky.model.numWires() == (numWires - 1), "Test wire deletion failed."

# Step 2: Delete the variable 'emprate'
emprate_coords = findVariable("emprate")
if emprate_coords:
    deleteKey(emprate_coords[0], emprate_coords[1])  # Delete the variable directly using coordinates
    assert minsky.model.numItems() == (numItems - 1), "Test 'emprate' variable deletion failed."
    # Two wires should also be deleted after deleting the variable
    assert minsky.model.numWires() == (numWires - 3), "Test attached wires deletion failed."
else:
    raise ValueError("Variable 'emprate' not found in the model.")

# Step 3: Delete the variable 'Investment'
investment_coords = findVariable("Investment")
if investment_coords:
    minsky.canvas.selectAllVariables()
    deleteKey(investment_coords[0], investment_coords[1])  # Delete the variable using coordinates
    assert minsky.model.numItems() == (numItems - 2), "Test selection deletion failed."
else:
    raise ValueError("Variable 'Investment' not found in the model.")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
