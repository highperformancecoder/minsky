#!../minsky 

# usage minsky compareFileLog <system.mky> <logfile>

# compares the running of a minsky model <system.mky>, with a previously
# run data log file <logfile>, created by "createFileLog"

source $minskyHome/library/init.tcl
use_namespace minsky
minsky.load $argv(2)
set log [open $argv(3) r]
# first line should specify no of steps to run
set nsteps [lindex [gets $log] 1]

proc fclose {x y} {
    if {abs($x)>1e-30} {
        return [expr abs($x-$y)/(abs($x)+abs($y)) < 1e-2]
    } else {
        return abs($x-$y) <= 1e-30
    }
}

# prepare element accessors for later use
foreach name [variables.values.#keys] {
        variables.values.@elem $name
}
use_namespace minsky
set status 0

for {set step 0} {$step<10} {incr step} {
    step
    gets $log logbuf

    if {![fclose [t] [lindex $logbuf 0]]} {
        puts "t=[t], logged [lindex $logbuf 0]"
        set status 1
    }

    array set values [lrange $logbuf 1 end]

    # note - we must use the .@elem form, to prevent '\' embedded in
    # variable names from being expanded
    foreach name [variables.values.#keys] {
        value.get $name
        if {![fclose [value.value] $values($name)]} {
            puts "$argv(2) t=[t], $name=[value.value], logged  $values($name)"
            set status 1
        }
    }
}

tcl_exit $status
