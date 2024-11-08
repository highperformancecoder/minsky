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
from pyminsky import minsky, findObject

# Load the specified .mky file
minsky.load('$here/examples/GoodwinLinear02.mky')

# Center the canvas
minsky.canvas.recentre()

# Find and focus on the Group item
group = findObject("Group")
assert group is not None, "Group not found or not focused"

# Retrieve initial dimensions and zoom
x = group.right()
y = group.bottom()
w = group.width()
h = group.height()
z = group.zoomFactor()
print(f"Group initial position and dimensions: right={x}, bottom={y}, width={w}, height={h}, zoom={z}")

# Perform mouse operations to resize the Group item
minsky.canvas.mouseDown(x, y)
x += 100
y += 100
minsky.canvas.mouseUp(x, y)

# Reselect the Group to verify updated dimensions
found_group = findObject("Group")
assert found_group is not None, "Could not reselect Group item after resize"

# Retrieve updated dimensions
updated_x = found_group.right()
updated_y = found_group.bottom()
print(f"Updated Group position: right={updated_x}, bottom={updated_y}")

# Verify that the updated position is within the expected bounds
assert abs(x - updated_x) < 6, "Right boundary mismatch after resizing"
assert abs(y - updated_y) < 6, "Bottom boundary mismatch after resizing"
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
