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

cat >input.tcl <<EOF
minsky.defaultFont Sans
minsky.setGodleyIconResource $here/gui-tk/icons/bank.svg
minsky.setGroupIconResource $here/gui-tk/icons/group.svg
minsky.load $here/test/allItems.mky
minsky.multipleEquities 1
minsky.canvas.renderToSVG allItemsBare.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items.@elem \$i
  minsky.model.items(\$i).mouseFocus 1
  minsky.model.items(\$i).tooltip tooltip
}
for {set i 0} {\$i<[minsky.model.groups.size]} {incr i} {
  minsky.model.groups.@elem \$i
  minsky.model.groups(\$i).mouseFocus 1
  minsky.model.groups(\$i).tooltip tooltip
}
minsky.canvas.renderToSVG allItemsMouseOver.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items(\$i).mouseFocus 0
  minsky.model.items(\$i).selected 1
}
for {set i 0} {\$i<[minsky.model.groups.size]} {incr i} {
  minsky.model.groups(\$i).mouseFocus 0
  minsky.model.groups(\$i).selected 1
}
minsky.canvas.renderToSVG allItemsSelected.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items(\$i).selected 0
  minsky.model.items(\$i).onBorder 1
}
minsky.canvas.renderToSVG allItemsOnBorder.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  if {[ minsky.model.items(\$i).classType]=="Sheet"} {
    minsky.model.items(\$i).showSlice "headAndTail"
    minsky.model.items(\$i).updateBoundingBox
  }
}
minsky.canvas.renderToSVG allItemsHeadAndTail.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  if {[ minsky.model.items(\$i).classType]=="Sheet"} {
    minsky.model.items(\$i).showSlice "tail"           
    minsky.model.items(\$i).updateBoundingBox
  }
}
minsky.canvas.renderToSVG allItemsTail.svg


tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

for i in *.svg; do
    # ids are assigned randomly, so strip out those tags
    sed -e 's/id="[^"]*"/id=""/' <$i >tmp1
    sed -e 's/id="[^"]*"/id=""/' <$here/test/renderedImages/$i >tmp2
    $here/test/compareSVG.sh tmp1 tmp2
    if [ $? -ne 0 ]; then fail; fi
done

pass
