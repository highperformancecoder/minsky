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
# check if rectangle A is contaned in rectangle B
proc contained {A B} {
  return [expr [lindex \$A 0]>=[lindex \$B 0] && [lindex \$A 0]<=[lindex \$B 2] && \
   [lindex \$A 1]>=[lindex \$B 1] && [lindex \$A 1]<=[lindex \$B 3] && \
   [lindex \$A 2]>=[lindex \$B 0] && [lindex \$A 2]<=[lindex \$B 2] && \
   [lindex \$A 3]>=[lindex \$B 1] && [lindex \$A 3]<=[lindex \$B 3]]
}

proc afterMinskyStarted {} {
  source "assert.tcl"
  minsky.load testGroup.mky
  group.get 33
# initially, no internal items should be visible
  foreach v [group.variables] {
    var.get \$v
    assert {[var.visible]==0} "var\$v"
    assert "\[.wiring.canvas find withtag var\$v\]=={}" "var\$v"
  }
  foreach o [group.operations] {
    op.get \$o
    assert {[op.visible]==0} "op\$o"
    assert "\[.wiring.canvas find withtag op\$o\]=={}" "op\$o"
  }
  foreach w [group.wires] {
    wire.get \$w
    assert {[wire.visible]==0} "wire\$w"
    assert "\[.wiring.canvas find withtag wire\$w\]=={}" "wire\$w"
  }
# zoom into displayZoom - everything should now be visible
  group::zoomToDisplay 33
  foreach v [group.variables] {
    var.get \$v
    if {[lsearch [group.edgeSet] \$v]==-1} {
      assert {[var.visible]==1} "var\$v"
      assert "\[llength \[.wiring.canvas find withtag var\$v\]\]==1" {}
      # icon should be within content bounds
      set varBBox [.wiring.canvas bbox var\$v]
      assert "\[contained \"\$varBBox\" \[.wiring.canvas bbox group33\]\]" {}
    } else {
      assert {[var.visible]==0} "edge var\$v"
      assert "\[llength \[.wiring.canvas find withtag var\$v\]\]==0" "edge var\$v"
    }
  }
  foreach o [group.operations] {
    op.get \$o
    assert {[op.visible]==1} "op\$o"
    assert "\[llength \[.wiring.canvas find withtag op\$o\]\]==1" "op\$o"
    # icon should be within content bounds
    set opBBox [.wiring.canvas bbox op\$o]
    assert "\[contained \"\$opBBox\" \[.wiring.canvas bbox group33\]\]" {}
  }
  foreach w [group.wires] {
    wire.get \$w
    assert {[wire.visible]==1} "wire\$w"
    assert "\[llength \[.wiring.canvas find withtag wire\$w\]\]==1" "wire\$w"
  }
# zoom back to original
 zoom [expr 1/[zoomFactor]]
  foreach v [group.variables] {
    var.get \$v
    assert {[var.visible]==0} "var\$v"
    assert "\[.wiring.canvas find withtag var\$v\]=={}" "var\$v"
  }
  foreach o [group.operations] {
    op.get \$o
    assert {[op.visible]==0} "op\$o"
    assert "\[.wiring.canvas find withtag op\$o\]=={}" "op\$o"
  }
  foreach w [group.wires] {
    wire.get \$w
    assert {[wire.visible]==0} "wire\$w"
    assert "\[.wiring.canvas find withtag wire\$w\]=={}" "wire\$w"
  }
 group.deleteContents
 updateCanvas
 # should now be only two variables left
 assert {[variables.size]==2} {}
 assert {[operations.size]==0} {}
 assert {[wires.size]==0} {}
 # enable exiting without the save dialog
 resetEdited
 exit
}
EOF

#disabled!
pass

cp $here/test/testGroup.mky .
cp $here/test/assert.tcl .
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
