#!../minsky

# usage minsky createFileLog <system.mky>

# writes a log file on stdout for system <system.mky>

source $minskyHome/library/init.tcl
use_namespace minsky
minsky.load $argv(2)


set nsteps 10
puts "nsteps 10"
running 1
#provide seed to ensure repeatability
srand 10
reset
#step

for {set step 0} {$step<$nsteps} {incr step} {
    step
    puts -nonewline "[t] "

    # note - we must use the .@elem form, to prevent '\' embedded in
    # variable names from being expanded

    foreach name [variableValues.#keys] {
        if [regexp "^constant:" $name] continue
        getValue "$name"
        if [value.temp] continue
        if {[value.size]>1} {
            for {set i 0} {$i<[value.size]} {incr i} {
                puts -nonewline "{$name|$i} [value.value $i] "
            }
        } else {
            puts -nonewline "{$name} [value.value] "
        }
    }
    puts ""
}
tcl_exit
