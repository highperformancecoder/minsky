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
    set godleyId [addGodleyTable]
    godley.get \$godleyId
    godley.moveTo 100 100
    godley.table.resize 3 2
    godley.setCell 0 1 foobar
    godley.setCell 2 1 bar
    godley.update
    wiringGroup.updateOnNewGodleyVars \$godleyId

    newItem \$godleyId
    bind .old_wiring.canvas <Button-1> "puts {%x %y}"

    # delivered to foobar
    event generate .old_wiring.canvas <Button-3>  -x 75 -y 150 -rootx 100 -rooty 100
    # check context menu is posted
    assert {[winfo viewable .old_wiring.context]} foobar
    # check the menu items are what is expected
    assert {[string match "Edit" "[.old_wiring.context entrycget 0 -label]"]} foobar
    assert {[.old_wiring.context entrycget 1 -label]=="Copy"} foobar


    .old_wiring.context unpost

    # delivered to bar
    event generate .old_wiring.canvas <Button-3> -x 55 -y 80 -rootx 100 -rooty 100
    assert [winfo viewable .old_wiring.context] bar
    assert {[string match "Edit" "[.old_wiring.context entrycget 0 -label]"]} bar
    assert {[.old_wiring.context entrycget 1 -label]=="Copy"} bar

    .old_wiring.context unpost

    # delivered to the Godley icon
    event generate .old_wiring.canvas <Button-3> -x 113 -y 69 -rootx 100 -rooty 100
    assert [winfo viewable .old_wiring.context] godley
    assert "\[.old_wiring.context entrycget 2 -command\]==\"openGodley \$godleyId\"" godley
    assert "\[.old_wiring.context entrycget 8 -command]\==\"deleteItem \$godleyId item\$godleyId\"" godley

    .old_wiring.context unpost
    # delivered to nowhere
    event generate .old_wiring.canvas <Button-3> -x 200 -y 200 -rootx 100 -rooty 100
    assert {![winfo viewable .old_wiring.context]} nowhere
    tcl_exit
}

EOF
$here/gui-tk/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
