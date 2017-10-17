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

# check description/tooltip functionality
cat >input.tcl <<EOF
source assert.tcl
proc afterMinskyStarted {} {
  minsky.addVariable foo flow
  deiconifyNote
  .wiring.note.tooltip.entry insert 0 foobar
  .wiring.note.text insert 1.0 "some longer text"
  OKnote itemFocus
  assert {"foobar"==[minsky.canvas.itemFocus.tooltip]}
  assert {"some longer text"==[minsky.canvas.itemFocus.detailedText]}
  minsky.save saved.mky
  tcl_exit
}
EOF

cp $here/test/assert.tcl .
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

# check that it reloads
cat >reload.tcl <<EOF
source assert.tcl
minsky.load saved.mky
assert {[minsky.findObject "Variable:flow"]}
assert {"foobar"==[minsky.canvas.item.tooltip]}
assert {"some longer text"==[minsky.canvas.item.detailedText]}
tcl_exit
EOF

$here/gui-tk/minsky reload.tcl
if test $? -ne 0; then fail; fi

pass
