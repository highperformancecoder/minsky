
proc assert {x args} {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed: $args"
        resetEdited
        tcl_exit 1
    }
}
