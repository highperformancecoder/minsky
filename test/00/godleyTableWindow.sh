#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
import sys
sys.path.insert(0,'$here')
from pyminsky import minsky, findObject
minsky.defaultFont('Sans')

minsky.load('$here/examples/1Free.mky')
minsky.multipleEquities(True)
minsky.displayStyle('sign')
minsky.displayValues(True)
item=findObject('GodleyIcon')
gw=item.popup
gw.enableButtons()
gw.renderToSVG('1FreeBase.svg')
gw.selectedCol(1)
gw.selectedRow(2)
gw.insertIdx(0)
gw.selectIdx(3)
gw.hoverRow(2)
gw.hoverCol(2)
gw.mouseMoveB1(gw.colLeftMargin()[2]+10, 4*gw.rowHeight()+gw.topTableOffset())
minsky.displayStyle('DRCR')
gw.renderToSVG('1Free11Selected.svg')
gw.selectedCol(0)
gw.renderToSVG('1FreeSelectedRow.svg')
gw.selectedCol(1)
gw.selectedRow(0)
gw.mouseMoveB1(gw.colLeftMargin[2]+10, 3*gw.rowHeight()+gw.topTableOffset())
gw.renderToSVG('1FreeSelectedCol.svg')
  
minsky.load('$here/examples/LoanableFunds.mky')
item=findObject('GodleyIcon')
minsky.displayStyle('DRCR')
minsky.displayValues(0)
item.popup.enableButtons()
item.popup.renderToSVG('LoanableFundsBase.svg')
EOF

python3 input.py
if [ $? -ne 0 ]; then fail; fi

for i in *.svg; do
    # ids are assigned randomly, so strip out those tags
    sed -e 's/id="[^"]*"/id=""/' <$i >tmp1
    sed -e 's/id="[^"]*"/id=""/' <$here/test/renderedImages/$i >tmp2
    $here/test/compareSVG.sh tmp1 tmp2
    if [ $? -ne 0 ]; then fail; fi
done

pass
