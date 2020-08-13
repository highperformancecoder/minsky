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
foreach name [variableValues.#keys] {
        variableValues.@elem $name
}
use_namespace minsky
set ret 0

running 1
#provide seed to ensure repeatability
srand 10

for {set step 0} {$step<10} {incr step} {

    step
    gets $log logbuf
    if {![fclose [t] [lindex $logbuf 0]]} {
        puts "t=[t], logged [lindex $logbuf 0]"
        set ret 1
    }


    array set values [lrange $logbuf 1 end]

    # note - we must use the .@elem form, to prevent '\' embedded in
    # variable names from being expanded
    foreach name [variableValues.#keys] {
        if [regexp "^constant:" $name] continue
        getValue $name

        # local variables can have a rather arbitrary scope
        # name. Check against all other variables with same name but
        # different scope

        if {[lsearch [array names values] $name]==-1} {
            #puts "$name ->[array names values *:[regsub "(.*:)" $name ""]]"
            set status 1
            foreach n [array names values *:[regsub "(.*:)" $name ""]] {
                #puts "[value.value] $values($n)"
                if [fclose [value.value] $values($n)] {
                    set status 0
                    break
                }
            }
            if {$status} {
                set ret 1
                puts "unable to find matching var $name"
                break
            }
            continue
        }
        
        if {![fclose [value.value] $values($name)]} {
            puts "$argv(2) t=[t], $name=[value.value], logged  $values($name)"
            set ret 1
        }
    }
}

tcl_exit $ret
