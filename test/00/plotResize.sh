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

# Function to simulate assertion behavior
def assert_condition(condition, message="Assertion failed!"):
    if not condition:
        raise AssertionError(message)

# Generalized function to find and return the specified item or group type on the canvas
def findObject(type_name):
    if type_name == "Group":
        # Access groups by index, as minsky.model.groups is not directly iterable
        for i in range(len(minsky.model.groups)):
            group = minsky.model.groups[0]
            if group.visible():
                minsky.canvas.getItemAt(group.x(), group.y())
                return minsky.canvas.item()
    else:
        # Otherwise, search through items
        for i in range(len(minsky.model.items)):
            item = minsky.model.items[i]
            if item.classType() == type_name and item.visible():
                minsky.canvas.getItemAt(item.x(), item.y())
                return minsky.canvas.item()
    return None

# Add a PlotWidget to the canvas
minsky.canvas.addPlot()
minsky.canvas.mouseUp(300, 200)  # Finalize the position

# Attempt to select the PlotWidget
plot_widget = findObject("PlotWidget")
assert_condition(plot_widget is not None, "PlotWidget not found or not focused")
print(f"Focused item: {plot_widget.classType()} at ({plot_widget.x()}, {plot_widget.y()})")

# Update the bounding box to initialize dimensions
plot_widget.updateBoundingBox()

# Retrieve initial dimensions and print for debugging
w = plot_widget.width()
h = plot_widget.height()
print(f"PlotWidget dimensions after update are ({w}, {h})")

# Perform assertions on width and height
assert_condition(w is not None and w > 100, "Width is not greater than 100 or is None")
assert_condition(h is not None and h > 100, "Height is not greater than 100 or is None")

# Calculate safe coordinates for the bottom-right resize handle
resize_x = plot_widget.right() + 5
resize_y = plot_widget.bottom() + 5
print(f"PlotWidget resize handle is located at ({resize_x}, {resize_y})")

# Simulate a mouse click on the resize handle
print("Performing mouseDown...")
minsky.canvas.mouseDown(resize_x, resize_y)

# Directly perform mouseUp to complete the resize
print("Performing mouseUp...")
minsky.canvas.mouseUp(resize_x + w, resize_y + h)

# Reselect the PlotWidget after resizing
found_plot_widget = findObject("PlotWidget")
assert_condition(found_plot_widget is not None, "Could not reselect PlotWidget after resize")
print(f"Focused item after reselecting: {found_plot_widget.classType()} at ({found_plot_widget.x()}, {found_plot_widget.y()})")

# Redraw the canvas to ensure it reflects the new size
minsky.canvas.redraw()

# Retrieve and print updated dimensions
updated_w = found_plot_widget.width()
updated_h = found_plot_widget.height()
print(f"Updated PlotWidget dimensions are ({updated_w}, {updated_h})")

# Verify that the updated dimensions are greater than the initial ones
assert_condition(abs((resize_x + w) - found_plot_widget.right()) < 5, "Right boundary mismatch after resizing")
assert_condition(abs((resize_y + h) - found_plot_widget.bottom()) < 5, "Bottom boundary mismatch after resizing")
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
