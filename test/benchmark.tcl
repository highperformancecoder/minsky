#!../GUI/minsky

proc afterMinskyStarted {} {
    global argv
    use_namespace minsky
    minsky.load $argv(2)
    updateCanvas
    enableEventProcessing
    for {set i 0} {$i<1000} {incr i} {
        set lastt [t]
        step
        .menubar.statusbar configure -text "t: [t] dt: [expr [t]-$lastt]"
        updateGodleysDisplay
        update
    }
    tcl_exit
}
