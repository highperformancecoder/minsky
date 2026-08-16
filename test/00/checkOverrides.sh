#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >checkOverrides.py <<EOF
from pyminsky import minsky
import sys

minsky.load(sys.argv[1])
minsky.doPushHistory(False)
numItems=len(minsky.model.items)

for i in range(numItems):
    if minsky.canvas.getItemAt(minsky.model.items[i].x(), minsky.model.items[i].y()):
        origClassType=minsky.canvas.item.classType()
        # removing GodleyIcon removes dependent variables from model
        if origClassType=="Item" or origClassType=="GodleyIcon": continue
        minsky.canvas.copyItem()
        minsky.canvas.mouseUp(-500, -500)
        assert minsky.canvas.getItemAt(-500, -500)
        if origClassType=="IntOp":
           assert "Variable:integral"==minsky.canvas.item.classType(),sys.argv[1]
        else:
           assert origClassType==minsky.canvas.item.classType(), sys.argv[1]
        minsky.canvas.deleteItem()
EOF

# this is really slow for some reason, so just check a selection of example files 
for i in $here/examples/{1Free,GoodwinLinear02,indexing,importedCSV}.mky; do
    echo $i
    python3 checkOverrides.py $i &>log
    if [ $? -ne 0 ]; then
        cat log
        fail
    fi
done

pass
