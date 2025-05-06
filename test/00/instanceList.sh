#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky, findObject, findVariable

minsky.load('$here/examples/GoodwinLinear02.mky')
findVariable("Y")
minsky.listAllInstances()
instanceList=minsky.variableInstanceList
assert len(instanceList.names())==2
offsx=minsky.model.x()
offsy=minsky.model.y()
instanceList.gotoInstance(0)
offsx0=minsky.model.x()
offsy0=minsky.model.y()
assert offsx0!=offsx
assert offsy0!=offsy

# offsets hard coded in VariableInstanceList
minsky.canvas.getItemAt(50, 50)
assert minsky.canvas.item.name()=="Y"

instanceList.gotoInstance(1)
assert minsky.model.x()!=offsx0
assert minsky.model.y()!=offsy0

minsky.canvas.getItemAt(50, 50)
assert minsky.canvas.item.name()=="Y"

offsx=minsky.model.x()
offsy=minsky.model.y()
id=minsky.canvas.item.id()
instanceList.gotoInstance(2)
# should do nothing
assert minsky.canvas.item.id()==id
assert minsky.model.x()==offsx
assert minsky.model.y()==offsy
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

pass
