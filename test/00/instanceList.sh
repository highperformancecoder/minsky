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

# Add the directory containing pyminsky.so to sys.path
sys.path.insert(0,'$here')

from pyminsky import minsky

def assert_condition(condition, message=""):
    if not condition:
        print(f"Assertion failed: {message}")
        sys.exit(1)
# Load the model
minsky.load('/home/dednaw/minsky/examples/GoodwinLinear02.mky')

# Get the number of items in the model
item_count = len(minsky.model.items)

# Find all variables named "Y"
variables_Y = []
for idx in range(item_count):
    item = minsky.model.items[idx]
    if item.classType().startswith("Variable:") and item.name() == "Y":
        variables_Y.append(item)

assert_condition(len(variables_Y) == 2, f"Expected 2 instances of 'Y', found {len(variables_Y)}")

# For each instance, get the item at its position and check it's correct
for idx, var_item in enumerate(variables_Y):
    # Get the item at var_item's position
    item_found = minsky.canvas.getItemAt(var_item.x(), var_item.y())
    assert_condition(item_found, f"No item found at variable's position for instance {idx}")
    # Check that the found item matches the expected item
    assert_condition(minsky.canvas.item().id() == var_item.id(),
                     f"Item at variable's position does not match expected item for instance {idx}")
    assert_condition(minsky.canvas.item().name() == "Y",
                     f"Item at variable's position is not 'Y' for instance {idx}")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
