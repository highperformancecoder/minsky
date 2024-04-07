#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# test that save file throws an error if it can't
cat >input.py <<EOF
import sys
sys.path.append('$here')
from pyminsky import minsky

minsky.save('writeable.mky')
try:
  minsky.save('/minsky-dummy/readonly.mky')
except:
  exit(0)
exit(1)
EOF

touch readonly.mky
chmod a-w readonly.mky
python3 input.py
if test $? -ne 0; then fail; fi

pass
