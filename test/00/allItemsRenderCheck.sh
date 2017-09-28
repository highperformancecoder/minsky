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
minsky.defaultFont Courier
minsky.load $here/test/allItems.mky
minsky.renderCanvasToSVG allItemsBare.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items.@elem \$i
  minsky.model.items(\$i).mouseFocus 1
  minsky.model.items(\$i).tooltip tooltip
}
minsky.renderCanvasToSVG allItemsMouseOver.svg

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items(\$i).mouseFocus 0
  minsky.model.items(\$i).selected 1
}
minsky.renderCanvasToSVG allItemsSelected.svg

tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

for i in *.svg; do
    diff -q $i $here/test/renderedImages/$i
    if [ $? -ne 0 ]; then fail; fi
done

pass
