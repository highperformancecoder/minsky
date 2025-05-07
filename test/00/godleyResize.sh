#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky, findObject

# Add a Godley table to the canvas
minsky.canvas.addGodley()
minsky.canvas.mouseUp(300, 200)  # Finalize the position

# Attempt to select the GodleyIcon
godley_icon = findObject("GodleyIcon")  
assert godley_icon is not None, "GodleyIcon not found or not focused"

# Print details about the focused item
print(f"Focused item: {godley_icon.classType()} at ({godley_icon.x()}, {godley_icon.y()})")

# Toggle editor mode to enable dimension adjustment
godley_icon.toggleEditorMode()

# Update the icon's bounding box to initialize dimensions
godley_icon.updateBoundingBox()

# Retrieve initial dimensions and print for debugging
w = godley_icon.width()
h = godley_icon.height()
print(f"GodleyIcon dimensions after update are ({w}, {h})")

# Perform assertions on width and height
assert w is not None and w > 100, "Width is not greater than 100 or is None"
assert h is not None and h > 100, "Height is not greater than 100 or is None"

# Calculate safe coordinates for the bottom-right resize handle
resize_x = godley_icon.right() + 5
resize_y = godley_icon.bottom() + 5
print(f"GodleyIcon resize handle is located at ({resize_x}, {resize_y})")

# Simulate a mouse click on the resize handle
#print("Performing mouseDown...")
minsky.canvas.mouseDown(resize_x, resize_y)

# Directly perform mouseUp to complete the resize
#print("Performing mouseUp...")
minsky.canvas.mouseUp(resize_x + w, resize_y + h)

# Reselect the GodleyIcon after resizing
found_godley_icon = findObject("GodleyIcon")
assert found_godley_icon is not None, "Could not reselect GodleyIcon after resize"
print(f"Focused item after reselecting: {found_godley_icon.classType()} at ({found_godley_icon.x()}, {found_godley_icon.y()})")

# Retrieve and print updated dimensions
updated_w = found_godley_icon.width()
updated_h = found_godley_icon.height()
print(f"Updated GodleyIcon dimensions are ({updated_w}, {updated_h})")

# Verify that the updated dimensions are greater than the initial ones
assert updated_w > w, "Width did not increase as expected"
assert updated_h > h, "Height did not increase as expected"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
