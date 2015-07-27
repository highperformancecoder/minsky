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
proc afterMinskyStarted {} {
  minsky.load $here/examples/1Free.mky
  recentreCanvas
  updateCanvas
  set godley [lindex [godleyItems.#keys] 0]
  doubleMouseGodley \$godley 194 57
  doubleMouseGodley \$godley 66 44
  update
  # should open edit window
  assert {[winfo viewable .wiring.editVar]}
  .wiring.editVar.buttonBar.ok invoke
  assert {![winfo viewable .wiring.editVar]}

  # double click on first variable
  set var [lindex [.wiring.canvas find withtag variables] 0]
  eval doubleClick \$var [.wiring.canvas coords \$var]
  assert {[winfo viewable .wiring.editVar]} {varclick}
  .wiring.editVar.buttonBar.ok invoke
  assert {![winfo viewable .wiring.editVar]} {varclick}


  resetEdited
  exit
}
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
