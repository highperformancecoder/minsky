#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

# Load the model
minsky.load('$here/examples/GoodwinLinear02.mky')

# Step 1: Access the first group directly from minsky.model.groups
group = minsky.model.groups[0]
assert group is not None, "No group found in the model."
print(f"Found group at coordinates ({group.x()}, {group.y()})")

# Step 2: Focus on the group item on the canvas
minsky.canvas.getItemAt(group.x(), group.y())
assert minsky.canvas.item.classType() == "Group", "Focused item is not a Group."

# Step 3: Open the group in the canvas
minsky.openGroupInCanvas()

# Step 4: Verify the number of items and wires in the group matches the canvas
group_model = group
canvas_model = minsky.canvas.model

assert group_model.numItems() == canvas_model.numItems(), "Number of items in the group and canvas do not match."
assert group_model.numWires() == canvas_model.numWires(), "Number of wires in the group and canvas do not match."

print("Group and canvas models match in terms of items and wires.")

# Step 5: Return to the main model in the canvas
minsky.openModelInCanvas()

# Step 6: Verify the number of items and wires in the main model matches the canvas
assert minsky.model.numItems() == minsky.canvas.model.numItems(), "Number of items in the main model and canvas do not match."
assert minsky.model.numWires() == minsky.canvas.model.numWires(), "Number of wires in the main model and canvas do not match."
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
