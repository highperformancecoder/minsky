minsky.load $argv(2)
minsky.reset
minsky.matlab $argv(2).m

# run the simulation for a while, and output the data values
set dat [open $argv(2).dat w]
puts $dat "d=\["

for {set i 0} {$i<7} {incr i} {
    puts -nonewline $dat "\[[minsky.t] "
    foreach var [minsky.variableValues.#keys] {
        minsky.getValue "$var"
        if {![minsky.value.isFlowVar]} {
            puts -nonewline $dat "[minsky.value.value] "
        }
    }
    puts $dat "\];"
    minsky.step
}
puts $dat "\];"
tcl_exit
