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
    set godleyId [addGodleyTable 100 100]
    godley.get \$godleyId
    godley.table.resize 3 2
    godley.setCell 0 1 foobar
    godley.setCell 2 1 bar
    godley.update
    godley.set

    newGodleyItem \$godleyId
    bind .wiring.canvas <Button-1> "puts {%x %y}"

    # delivered to foobar
    event generate .wiring.canvas <Button-3>  -x 75 -y 150 -rootx 100 -rooty 100
    # check context menu is posted
    assert {[winfo viewable .wiring.context]} foobar
    # check the menu items are what is expected
    assert {[string match "editItem * var" "[.wiring.context entrycget 0 -command]"]} foobar
    assert {[.wiring.context entrycget 1 -label]=="Copy"} foobar


    .wiring.context unpost

    # delivered to bar
    event generate .wiring.canvas <Button-3> -x 55 -y 80 -rootx 100 -rooty 100
    assert [winfo viewable .wiring.context] bar
    assert {[string match "editItem * var" "[.wiring.context entrycget 0 -command]"]} bar
    assert {[.wiring.context entrycget 1 -label]=="Copy"} bar

    .wiring.context unpost

    # delivered to the Godley icon
    event generate .wiring.canvas <Button-3> -x 113 -y 69 -rootx 100 -rooty 100
    assert [winfo viewable .wiring.context] godley
    assert "\[.wiring.context entrycget 2 -command\]==\"openGodley \$godleyId\"" godley
    assert "\[.wiring.context entrycget 7 -command]\==\"deleteItem \$godleyId godley\$godleyId\"" godley

    .wiring.context unpost
    # delivered to nowhere
    event generate .wiring.canvas <Button-3> -x 200 -y 200 -rootx 100 -rooty 100
    assert {![winfo viewable .wiring.context]} nowhere
    resetEdited
    exit
}

EOF
$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
