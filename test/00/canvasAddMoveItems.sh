#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# needs to be a jest test!
cat >input.py <<EOF
from pyminsky import minsky

# Clear the canvas to start with a clean slate
minsky.model().clear()

# Step 1: Check the canvas background context menu
def canvasContextBackground(x, y, X, Y):
    # First, check if any item exists at (x, y)
    item = minsky.canvas.getItemAt(x, y)
    
    if item is not None:
        item = minsky.canvas.itemFocus()
        if item is not None:
            minsky.canvas.deleteItem()
        else:
            pass
    
    # Verify the item has been fully removed
    item = minsky.canvas.getItemAt(x, y)
    if item is not None:
        item = minsky.canvas.itemFocus()
        if item is not None:
            assert not item.classType().startswith("Variable"), "Canvas should not have an interactive item selected on background right-click."
            minsky.canvas.deleteItem()

# Step 2: Simulate background context click
canvasContextBackground(0, 0, 0, 0)

# Step 3: Add and test variable context menu
minsky.canvas.addVariable('foo', 'flow')
item = minsky.canvas.itemFocus

# Move the variable to a visible and accessible position (50, 50)
item.moveTo(50, 50)
x, y = item.x(), item.y()

# Check context menu on the variable
def canvasContextOnItem(x, y, X, Y):
    # Try to get the item at these coordinates
    assert minsky.canvas.getItemAt(x, y), "Item not found at expected position."
    item = minsky.canvas.itemFocus()  # Use itemFocus to get the currently focused item
    assert item is not None, "An item should be selected when right-clicking on it."
    minsky.canvas.deleteItem()

canvasContextOnItem(x, y, int(x), int(y))

# Step 4: Add and test operation context menu
minsky.canvas.addOperation('integrate')
item.moveTo(100, 100)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 5: Add and test data operation context menu
minsky.canvas.addOperation('data')
item.moveTo(150, 150)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 6: Add and test plot context menu
minsky.canvas.addPlot()
item.moveTo(200, 200)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 7: Add and test Godley table context menu
minsky.canvas.addGodley()
item.moveTo(250, 250)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 8: Add and test group context menu
minsky.canvas.addGroup()
item.moveTo(300, 300)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 9: Add and test note context menu
minsky.canvas.addNote("hello")
item.moveTo(350, 350)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))

# Step 10: Add and test switch context menu
minsky.canvas.addSwitch()
item.moveTo(400, 400)
x, y = item.x(), item.y()
canvasContextOnItem(x, y, int(x), int(y))
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
