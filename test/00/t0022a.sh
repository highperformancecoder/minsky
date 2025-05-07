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
