#! /bin/sh

# test that a simple integral example works when both coupled and uncoupled

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky, findObject

# Step 1: Function to access value by ID
def getValue(valueId):
    try:
        # Access valueId directly via minsky's interface
        value = minsky.variableValues[valueId]
        print(f"Found variable {valueId} with value: {value.value()}")
        return value
    except KeyError:
        raise ValueError(f"Variable '{valueId}' not found in variableValues.")
    except Exception as e:
        raise ValueError(f"Error accessing variable '{valueId}': {e}")

# Load the model
minsky.load('$here/examples/exponentialGrowth.mky')

# Step 2: Verify initial value of ':y'
y_value = getValue(":y")
assert y_value.value() == 1, f"Expected initial value of :y to be 1, but got {y_value.value()}"

# Step 3: Find and check VarConstant
findObject("VarConstant")
var_constant_value = minsky.canvas.item.value()
assert var_constant_value != 0, "VarConstant item value is unexpectedly 0."
print(f"VarConstant value: {var_constant_value}")

# Step 4: Advance simulation and verify updated ':y'
def step():
    """Advance the simulation by one step."""
    minsky.step()

# Run two simulation steps
minsky.running = True
minsky.reset()
step()
step()

y_value_after_step = getValue(":y")
assert y_value_after_step.value() > 0, "Value of :y after steps is not greater than 0."

# Conditional check based on VarConstant value
if var_constant_value < 0:
    assert 0 < y_value_after_step.value() < 1, "Expected :y to be between 0 and 1 for negative VarConstant."
else:
    assert y_value_after_step.value() > 1, "Expected :y to be greater than 1 for positive VarConstant."

stored_y_value = y_value_after_step.value()

# Step 5: Toggle coupling for IntOp
findObject("IntOp")
minsky.canvas.item.toggleCoupled()
print("Toggled coupled state for IntOp.")

# Step 6: Reset and rerun simulation
minsky.reset()
step()
step()

y_value_after_toggle = getValue(":y")
if var_constant_value < 0:
    assert 0 < y_value_after_toggle.value() < 1, "Expected :y to be between 0 and 1 for negative VarConstant."
else:
    assert y_value_after_toggle.value() > 1, "Expected :y to be greater than 1 for positive VarConstant."

assert y_value_after_toggle.value() == stored_y_value, "Value of :y does not match after toggling coupled state."
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
