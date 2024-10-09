#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# needs to be a jest test!
cat >input.py <<EOF
import sys

sys.path.insert(0,'$here')

from pyminsky import minsky

def assert_condition(condition, message=""):
    if not condition:
        print(f"Assertion failed: {message}")
        sys.exit(1)

# Clear the model
minsky.model.clear()

# List of items to test
items_to_test = [
    ('Variable', 'foo', 'flow', 'Variable:flow'),
    ('Operation', 'integrate', None, 'IntOp'),
    ('Operation', 'data', None, 'DataOp'),
    ('Plot', None, None, 'PlotWidget'),
    ('Godley', None, None, 'GodleyIcon'),
    ('Group', None, None, 'Group'),
    ('Note', 'hello', None, 'Item'),
    ('Switch', None, None, 'SwitchIcon'),
]

# Keep track of added items for verification
added_items = []

for item_type, name, var_type, expected_class in items_to_test:
    # Add the item
    if item_type == 'Variable':
        minsky.canvas.addVariable(name, var_type)
    elif item_type == 'Operation':
        minsky.canvas.addOperation(name)
    elif item_type == 'Plot':
        minsky.canvas.addPlot()
    elif item_type == 'Godley':
        minsky.canvas.addGodley()
    elif item_type == 'Group':
        minsky.canvas.addGroup()
    elif item_type == 'Note':
        minsky.canvas.addNote(name)
    elif item_type == 'Switch':
        minsky.canvas.addSwitch()
    else:
        print(f"Unknown item type: {item_type}")
        sys.exit(1)
    
    # The newly added item should be the item in focus
    item = minsky.canvas.itemFocus()
    assert_condition(item is not None, f"{item_type} not added correctly")
    assert_condition(item.classType() == expected_class,
                     f"{item_type} class type mismatch: expected {expected_class}, got {item.classType()}")
    # Store the item for further checks
    added_items.append(item)

# Verify that added items are present in the model
# Get the number of items in the model
item_count = len(minsky.model.items)

# Collect IDs of items in model
model_item_ids = [minsky.model.items[idx].id() for idx in range(item_count)]

# Verify that all added items are in the model
for item in added_items:
    assert_condition(item.id() in model_item_ids, f"Item with ID {item.id()} not found in model items")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
