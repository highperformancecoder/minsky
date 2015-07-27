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
 updateCanvas
 set gid [lindex [groupItems.#keys] 0]
 group.get \$gid
 set x [group.x]
 set y [group.y]
 set w [group.width]
 set h [group.height]
 group::resize \$gid

 variable group::orig_width
 variable group::orig_height

 group::resizeRect resizeBBox [expr \$x+\$w]  [expr \$y+\$h]
 group::resizeItem resizeBBox \$gid  [expr \$x+151]  [expr \$y+151]
 assert "\$x==\[group.x\]"
 assert "\$y==\[group.y\]"
# approximate here, because group is rotated by pi, which is  numerically approximate
 assert {abs(300-[group.width])<6}
 assert {abs(300-[group.height])<6}
 resetEdited
 exit
}}
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
