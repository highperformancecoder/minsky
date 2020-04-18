#! /bin/sh

# test that selection of variables within the Godley table works

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
source $here/test/assert.tcl

# arrange for the following code to executed after Minsky has started.
proc afterMinskyStarted {} {
    enableEventProcessing
    # remove Destroy binding as it interferes with assert
    bind . <Destroy> {}

    # create godley table with an input and output variable
    addGodley
    assert {[findObject "GodleyIcon"]} {}
    minsky.canvas.item.moveTo 100 100
    minsky.canvas.item.table.resize 3 2
    minsky.canvas.item.setCell 0 1 foobar
    minsky.canvas.item.setCell 2 1 bar
    minsky.canvas.item.update

    bind .wiring.canvas <Button-1> "puts {%x %y}"

    # delivered to foobar
    findVariable foobar
    event generate .wiring.canvas <Button-3>  -x [minsky.canvas.item.x] -y [minsky.canvas.item.y] -rootx 100 -rooty 100
    # check context menu is posted
    assert {[winfo viewable .wiring.context]} foobar
    # check the menu items are what is expected
    assert {[.wiring.context entrycget 0 -label]=="Copy"} foobar


    .wiring.context unpost

    # delivered to bar
    findVariable bar
    event generate .wiring.canvas <Button-3>  -x [minsky.canvas.item.x] -y [minsky.canvas.item.y] -rootx 100 -rooty 100
    assert [winfo viewable .wiring.context] bar
    assert {[.wiring.context entrycget 0 -label]=="Copy"} bar

    .wiring.context unpost

    # delivered to the Godley icon
    findObject GodleyIcon
    # put the event in upper right quadrant to ensure we've got the icon
    event generate .wiring.canvas <Button-3>  \
    -x [expr [minsky.canvas.item.x]+0.4*[minsky.canvas.item.width]] \
    -y [expr [minsky.canvas.item.y]-0.4*[minsky.canvas.item.height]] \
    -rootx 100 -rooty 100
    assert [winfo viewable .wiring.context] godley
    assert "\[.wiring.context entrycget 2 -command\]==\"openGodley \[minsky.openGodley\]\"" godley
    assert "\[.wiring.context entrycget 12 -command]\==\"canvas.deleteItem\"" godley

    .wiring.context unpost
    # delivered to nowhere
    event generate .wiring.canvas <Button-3> -x 200 -y 200 -rootx 100 -rooty 100
    assert {![winfo viewable .wiring.context]} nowhere
    tcl_exit
}

EOF
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
