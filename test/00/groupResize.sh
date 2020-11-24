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
source $here/test/assert.tcl
proc afterMinskyStarted {} {uplevel #0 {
 minsky.load $here/examples/GoodwinLinear02.mky
 recentreCanvas
 set item minsky.canvas.item
 assert {[findObject Group]}
 set x [\$item.right]
 set y [\$item.bottom]
 set w [\$item.width]
 set h [\$item.height]
 set z [\$item.zoomFactor]

 canvas.mouseDown \$x \$y
 set x [expr \$x+100]
 set y [expr \$y+100]
 canvas.mouseUp \$x  \$y

 findObject Group
 assert "abs(\$x-[minsky.canvas.item.right])<6"
 assert "abs(\$y-[minsky.canvas.item.bottom])<6"
 tcl_exit
}}
EOF

$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
