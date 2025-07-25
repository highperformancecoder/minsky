#! /bin/sh

here=`pwd`
. test/common-test.sh

# check description/tooltip functionality
cat >input.py <<EOF
from pyminsky import minsky
minsky.canvas.addVariable('foo','flow')
minsky.canvas.itemFocus.tooltip('foobar')
minsky.canvas.itemFocus.detailedText('some longer text')
assert "foobar"==minsky.canvas.itemFocus.tooltip()
assert "some longer text"==minsky.canvas.itemFocus.detailedText()
minsky.save('saved.mky')
EOF

python3 input.py
if test $? -ne 0; then fail; fi

# check that it reloads
cat >reload.py <<EOF
from pyminsky import minsky
minsky.load('saved.mky')
assert len(minsky.model.items)==1
item=minsky.model.items[0]
assert "foobar"==item.tooltip()
assert "some longer text"==item.detailedText()
EOF

python3 reload.py
if test $? -ne 0; then fail; fi

pass
