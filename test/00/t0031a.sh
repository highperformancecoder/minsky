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
 update

 set gid [lindex [groupItems.#keys] 0]
 #create an op, and move it into the group
 group.get \$gid
 set id [addOperation time]

 event generate .wiring.canvas <Button-1> -x [group.x]  -y [group.y]

 # do the same thing with a variable
 

 op.get \$id
 assert "\[op.group\]==\$gid"

 move op \$id 1000 1000
 checkAddGroup op \$id  1000 1000
 assert "\[op.group\]!=\$gid"


 set id [lindex [variables.visibleVariables] 0]
 copyVar \$id
 event generate .wiring.canvas <Button-1> -x [group.x]  -y [group.y]
 assert "\[var.group\]==\$gid"

 move var \$id 1000 1000
 checkAddGroup var \$id  1000 1000
 assert "\[var.group\]!=\$gid"

 set x [group.x]
 set y [group.y]
 set newGroupId [insertGroupFromFile $here/examples/GoodwinLinear02.mky]
 insertNewGroup \$newGroupId
 event generate .wiring.canvas <Button-1> -x \$x  -y \$y
 
 group.get \$newGroupId
 assert "\[group.parent\]==\$gid"

 move group \$newGroupId 1000 1000
 checkAddGroup group \$newGroupId  1000 1000
 assert "\[group.group\]!=\$gid"

 set numGroups [groupItems.size]
 group::copy \$gid
 event generate .wiring.canvas <Button-1> -x 1000  -y 1000
 assert "\$numGroups==[expr [groupItems.size]-1]"


 groupEdit \$gid
 update
 assert {[winfo viewable .wiring.editGroup]}
 .wiring.editGroup.buttonBar.ok invoke
 assert {![winfo viewable .wiring.editGroup]}

 resetEdited
 exit
}}
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
