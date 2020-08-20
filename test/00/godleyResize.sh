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

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
proc bgerror x {
  puts \$x
  exit 1
}

source $here/test/assert.tcl
proc afterMinskyStarted {} {uplevel #0 {
 addGodley
 canvas.mouseUp 300 200
 findObject GodleyIcon
 set item minsky.canvas.item
 \$item.updateBoundingBox
# add 5 pixels to avoid clicking on a port
 set x [expr [\$item.right]+5]
 set y [expr [\$item.bottom]+5]
 set w [\$item.width]
 set h [\$item.height]
 assert "\$w>100"
 assert "\$h>100"
 canvas.mouseDown \$x \$y
 canvas.mouseUp [expr \$x+\$w]  [expr \$y+\$h]

 assert "abs([expr \$x+\$w]-[minsky.canvas.item.right])<10"
 assert "abs([expr \$y+\$h]-[minsky.canvas.item.bottom])<10"

 tcl_exit

}}
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
