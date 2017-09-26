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
minsky.addOperation add
minsky.canvas.itemFocus.moveTo 100 100
minsky.canvas.itemFocus.rotation 30
minsky.canvas.itemFocus.mouseFocus 1
minsky.addVariable flowVar flow 
minsky.canvas.itemFocus.moveTo 200 200
minsky.canvas.itemFocus.rotation 30
minsky.canvas.itemFocus.mouseFocus 1
minsky.renderCanvasToSVG mouseOver.svg

minsky.canvas.itemFocus.tooltip "a var"
minsky.getItemAt 100 100
minsky.canvas.item.tooltip "plus sign"
minsky.renderCanvasToSVG mouseOverTooltip.svg

minsky.canvas.itemFocus.mouseFocus 0
minsky.canvas.itemFocus.selected 1
minsky.canvas.item.mouseFocus 0
minsky.canvas.item.selected 1
minsky.renderCanvasToSVG selected.svg

tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

for i in *.svg; do
    diff -q $i $here/test/renderedImages/$i
    if [ $? -ne 0 ]; then fail; fi
done

pass
