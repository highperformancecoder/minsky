#! /bin/sh

here=`pwd`
. $here/test/common-test.sh
# This test exercises the commandHook functionality in RESTService

python3 >output <<EOF
import sys
sys.path.append('$here')
from pyminsky import minsky

minsky.pushHistory()
assert not minsky.edited()
minsky.nSteps()
assert not minsky.edited()
minsky.nSteps(10)
assert minsky.edited()
minsky.load("$here/examples/GoodwinLinear02.mky")
assert not minsky.edited()
minsky.save("foo.mky")
assert not minsky.edited()
minsky.canvas.addOperation("add")
assert minsky.edited()
minsky.save("foo.mky")
assert not minsky.edited()
EOF
if [ $? -ne 0 ]; then fail; fi

pass
