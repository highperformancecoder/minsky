#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.load('$here/examples/GoodwinLinear.mky')
minsky.logVarList(minsky.variableValues.keys())
minsky.openLogFile('tmp1.dat')
nsteps=10
with open('tmp2.dat', 'w') as out:
     out.write("#time")
     for name in minsky.variableValues.keys():
         out.write(' '+minsky.variableValues[name].name())
     out.write('\n')

     minsky.running(True)
     for step in range(nsteps):
         minsky.step()
         out.write('%g'%minsky.t())
         for name in minsky.variableValues.keys():
             out.write(' %g'%minsky.variableValues[name].value(0))
         out.write('\n')

minsky.reset() #closes tmp1.dat
EOF

python3 input.py
if test $? -ne 0; then fail; fi

diff tmp1.dat tmp2.dat
if test $? -ne 0; then fail; fi

pass
