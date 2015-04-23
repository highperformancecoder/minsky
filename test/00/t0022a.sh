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
  set id [newVariable foo flow]
  newVar \$id
  deiconifyNote
  .wiring.note.tooltip.entry insert 0 foobar
  .wiring.note.text insert 1.0 "some longer text"
  OKnote var \$id
  var.get \$id
  assert {"foobar"==[var.tooltip]}
  assert {"some longer text\n"==[var.detailedText]}
  itemEnterLeave var \$id var\$id 1
  # check for existence of tooltip
  assert {[llength [.wiring.canvas find withtag tooltip]]==1}
  assert {[.wiring.canvas itemcget tooltip -text]=="foobar"}
  minsky.save saved.mky
  resetEdited
  exit
}
EOF

cp $here/test/assert.tcl .
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

# check that it reloads
cat >reload.tcl <<EOF
source assert.tcl
minsky.load saved.mky
set id [lindex [minsky.variables.#keys] 0]
minsky.var.get \$id
assert {"foobar"==[minsky.var.tooltip]}
assert {"some longer text\n"==[minsky.var.detailedText]}
tcl_exit
EOF

$here/GUI/minsky reload.tcl
if test $? -ne 0; then fail; fi

pass
