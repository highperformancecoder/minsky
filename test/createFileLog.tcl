#!../minsky

# usage minsky createFileLog <system.mky>

# writes a log file on stdout for system <system.mky>

source $minskyHome/library/init.tcl
use_namespace minsky
minsky.load $argv(2)

# prepare element accessors for later use
foreach name [variables.values.#keys] {
    variables.values.@elem $name
}
use_namespace minsky

set nsteps 10
puts "nsteps 10"
for {set step 0} {$step<$nsteps} {incr step} {
    step
    puts -nonewline "[t] "

    # note - we must use the .@elem form, to prevent '\' embedded in
    # variable names from being expanded

    foreach name [variables.values.#keys] {
        value.get "$name"
        puts -nonewline "{$name} [value.value] "
    }
    puts ""
}
tcl_exit
