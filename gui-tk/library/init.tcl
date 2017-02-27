proc use {name} {
    regsub {^[^.]*\.} $name "" procname
# don't overwrite existing commands
    if {[llength [info commands $procname]]==0} {
	proc $procname {args} "eval $name \$args"
    } else {
#        puts stderr "$procname already exists in target namespace, not exported"
    }
}

proc savebgerror {} {
    if {![llength [info commands obgerror]]} {
        if {[llength [info commands bgerror]]} {rename bgerror obgerror}
        proc bgerror x {puts stdout $x}
    }
}

proc restorebgerror {} {
    if {[llength [info commands obgerror]]} {
	proc bgerror {} {}  
	rename bgerror {}
	rename obgerror bgerror
    }
}

proc use_namespace {name} {
    foreach comm [info commands [set name].*] {use $comm}
}

# if unknown procedure has () in it, attempt to call set/get methods
catch {rename unknown tclunknown}
proc unknown {procname args} {
    if [regexp "(.*)\\((.*)\\)\\..*" $procname wholestring cmd index] {
# attempt to create an object using get, and recall original command
	$cmd.get $index  
	$procname $args
    }
    if [regexp "(.*)\\((.*)\\)" $procname wholestring cmd index] {
	if [llength [info commands $cmd.get]] {
	    if {[llength $args]==0} {
		return [eval $cmd.get [split $index ,]]
	    } else {
		return [eval $cmd.set [split $index ,] $args]
	    }
	}
    }
    # otherwise try prefixing with minsky
    if [llength [info commands minsky.$procname]] {return [eval minsky.$procname $args]}
    return [eval tclunknown $procname $args]
}

