#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky, findObject

# Add a PlotWidget to the canvas
minsky.canvas.addPlot()
minsky.canvas.mouseUp(300, 200)  # Finalize the position

# Attempt to select the PlotWidget
plot_widget = findObject("PlotWidget")   
assert plot_widget is not None, "PlotWidget not found or not focused"
print(f"Focused item: {plot_widget.classType()} at ({plot_widget.x()}, {plot_widget.y()})")

# Update the bounding box to initialize dimensions
plot_widget.updateBoundingBox()

# Retrieve initial dimensions and print for debugging
w = plot_widget.width()
h = plot_widget.height()
print(f"PlotWidget dimensions after update are ({w}, {h})")

# Perform assertions on width and height
assert w is not None and w > 100, "Width is not greater than 100 or is None"
assert h is not None and h > 100, "Height is not greater than 100 or is None"

# Calculate safe coordinates for the bottom-right resize handle
resize_x = plot_widget.right() + 5
resize_y = plot_widget.bottom() + 5
print(f"PlotWidget resize handle is located at ({resize_x}, {resize_y})")

# Simulate a mouse click on the resize handle
#print("Performing mouseDown...")
minsky.canvas.mouseDown(resize_x, resize_y)

# Directly perform mouseUp to complete the resize
#print("Performing mouseUp...")
minsky.canvas.mouseUp(resize_x + w, resize_y + h)

# Reselect the PlotWidget after resizing
found_plot_widget = findObject("PlotWidget")
assert found_plot_widget is not None, "Could not reselect PlotWidget after resize"
print(f"Focused item after reselecting: {found_plot_widget.classType()} at ({found_plot_widget.x()}, {found_plot_widget.y()})")

# Retrieve and print updated dimensions
updated_w = found_plot_widget.width()
updated_h = found_plot_widget.height()
print(f"Updated PlotWidget dimensions are ({updated_w}, {updated_h})")

# Verify that the updated dimensions are greater than the initial ones
assert abs((resize_x + w) - found_plot_widget.right()) < 5, "Right boundary mismatch after resizing"
assert abs((resize_y + h) - found_plot_widget.bottom()) < 5, "Bottom boundary mismatch after resizing"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
