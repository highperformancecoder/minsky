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

cat >cut-paste.tcl <<EOF
source assert.tcl
proc afterMinskyStarted {} {

minsky.load $here/examples/GoodwinLinear02.mky
recentreCanvas
assert {[model.numItems]==26} {}
assert {[model.numWires]==27} {}
canvas.mouseDown 378 14
canvas.mouseUp  450 106
assert {[canvas.selection.numItems]==3} {}
assert {[canvas.selection.numWires]==2} {}
cut
assert {[model.numItems]==23} {}
assert {[model.numWires]==23} {}
paste

event generate .wiring.canvas <Button-1>

assert {[model.numItems]==26} {}
assert {[model.numWires]==25} {}

# find a wire with internal control points
for {set i 0} {\$i<[model.wires.size]} {incr i} {
  model.wires.@elem [set i]
  if {[llength [minsky.model.wires(\$i).coords]]>4} {
    set wire minsky.model.wires(\$i)
    break
  }
}

\$wire.straighten
assert "[llength [[set wire].coords]]==4" {llength wire.coords==4}

# some code that prints the values used in the next wiring op
#for {set item 0} {\$item<[model.items.size]} {incr item} {
#  minsky.model.items.@elem \$item
#  if {[minsky.model.items(\$item).classType]=="Operation:divide"} {
#    minsky.model.items(\$item).ports.@elem 0
#    puts "div: [minsky.model.items(\$item).ports(0).x] [minsky.model.items(\$item).ports(0).y]"
#  }
#  if {[minsky.model.items(\$item).classType]=="Variable:flow" && [minsky.model.items(\$item).name]=="emprate"} {
#    minsky.model.items(\$item).ports.@elem 1
#    puts "emprate: [minsky.model.items(\$item).ports(1).x] [minsky.model.items(\$item).ports(1).y]"
#  }
#}



# add another wire
canvas.mouseDown 363 43
canvas.mouseUp 486 173
assert {[model.numWires]==26} {}

tcl_exit
}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky cut-paste.tcl
if test $? -ne 0; then fail; fi

pass
