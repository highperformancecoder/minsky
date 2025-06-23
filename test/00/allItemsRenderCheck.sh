#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

cat >input.py <<EOF
from pyminsky import minsky

minsky.defaultFont('Sans')
minsky.setGodleyIconResource('$here/gui-js/apps/minsky-electron/src/assets/godley.svg')
minsky.setGroupIconResource('$here/gui-js/apps/minsky-electron/src/assets/group.svg')
minsky.setRavelIconResource('$here/gui-js/apps/minsky-electron/src/assets/ravel-logo.svg')
minsky.setLockIconResource('$here/gui-js/apps/minsky-electron/src/assets/locked.svg','$here/gui-js/apps/minsky-electron/src/assets/unlocked.svg')
minsky.histogramResource.setResource('$here/gui-js/apps/minsky-electron/src/assets/histogram.svg')

minsky.load('$here/test/allItems.mky')
minsky.multipleEquities(1)
minsky.canvas.renderToSVG('allItemsBare.svg')

for i in range(len(minsky.model.items)):
    minsky.model.items[i].mouseFocus(True)
    minsky.model.items[i].tooltip('tooltip')

for i in range(len(minsky.model.groups)):
    minsky.model.groups[i].mouseFocus(1)
    minsky.model.groups[i].tooltip('tooltip')

minsky.canvas.renderToSVG('allItemsMouseOver.svg')

for i in range(len(minsky.model.items)):
  minsky.model.items[i].mouseFocus(False)
  minsky.model.items[i].selected(True)

for i in range(len(minsky.model.groups)):
  minsky.model.groups[i].mouseFocus(False)
  minsky.model.groups[i].selected(True)

minsky.canvas.renderToSVG('allItemsSelected.svg')

for i in range(len(minsky.model.items)):
  minsky.model.items[i].selected(False)
  minsky.model.items[i].onBorder(True)

minsky.canvas.renderToSVG('allItemsOnBorder.svg')

for i in range(len(minsky.model.items)):
  if minsky.model.items[i].classType() =="Sheet":
    minsky.model.items[i].showRowSlice("headAndTail")
    minsky.model.items[i].updateBoundingBox()

minsky.canvas.renderToSVG('allItemsHeadAndTail.svg')

for i in range(len(minsky.model.items)):
  if minsky.model.items[i].classType()=="Sheet":
    minsky.model.items[i].showRowSlice("tail")           
    minsky.model.items[i].updateBoundingBox()

minsky.canvas.renderToSVG('allItemsTail.svg')

for i in range(len(minsky.model.items)):
  if minsky.model.items[i].classType() =="Sheet":
    minsky.model.items[i].showColSlice("headAndTail")
    minsky.model.items[i].updateBoundingBox()

minsky.canvas.renderToSVG('allItemsColHeadAndTail.svg')

for i in range(len(minsky.model.items)):
  if minsky.model.items[i].classType()=="Sheet":
    minsky.model.items[i].showColSlice("tail")           
    minsky.model.items[i].updateBoundingBox()

minsky.canvas.renderToSVG('allItemsColTail.svg')
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
