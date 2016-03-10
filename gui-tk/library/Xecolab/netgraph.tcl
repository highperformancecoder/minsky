proc netgraph {name args} {
    if {![winfo exists .$name]} {
	namespace eval $name {
	    variable name [string trimleft [namespace current] :]
	    image create photo .$name.netimg 
	    toplevel .$name 
	    canvas .$name.canvas -background white
	    .$name.canvas create image 0 0 -anchor nw -image .$name.netimg -tag netimg 

	    frame .$name.buttons
	    radiobutton .$name.buttons.dot \
		-value dot -variable .$name.graphviz_cmd -text "dot  " \
		-command [namespace code refresh] 
	    radiobutton .$name.buttons.neato \
		-value neato -variable .$name.graphviz_cmd -text neato \
		-command [namespace code refresh] 
	    radiobutton .$name.buttons.twopi \
		-value twopi -variable .$name.graphviz_cmd -text twopi \
		-command [namespace code refresh] 
	    radiobutton .$name.buttons.circo \
		-value circo -variable .$name.graphviz_cmd -text circo \
		-command [namespace code refresh] 
	    radiobutton .$name.buttons.fdp \
		-value fdp -variable .$name.graphviz_cmd -text "fdp  " \
		-command [namespace code refresh] 
	    .$name.buttons.dot select
	    pack .$name.buttons.dot .$name.buttons.neato .$name.buttons.twopi \
		.$name.buttons.circo .$name.buttons.fdp
	    pack .$name.canvas .$name.buttons -side left 
	    file delete -force .netimg$name

	    update
	    set offx [expr [winfo width .$name]-[.$name.canvas cget -width]]
	    set offy [expr [winfo height .$name]-[.$name.canvas cget -height]]
	    bind .$name <Configure> ".$name.canvas configure -height \[expr \[winfo height .$name\]-$offy\] -width \[expr \[winfo width .$name\]-$offx\]"

	    proc refresh {} {
		variable name
		variable last_data 
		global .$name.graphviz_cmd
		if {[
                     catch {
		    exec [set .$name.graphviz_cmd] -Tgif -o.netimg$name \
			>&.netimgerror <<$last_data
                     } results]
                } {
                    puts stderr $results
                    # display graphviz's stderror to the console
                    exec cat .netimgerror
                }
		.$name.netimg read .netimg$name -shrink
	    }

	    proc netgraph {args} {
		variable name
		variable last_data

		while [string equal -length 1 [lindex $args 0] "-"] {
		    if {[lindex $args 0]=="-style"} {
			set graphviz_cmd [lindex $args 1]
			set args [lreplace $args 0 1]
		    }

		    if {[lindex $args 0]=="-title"} {
			wm title .$name [lindex $args 1]
			set args [lreplace $args 0 1]
		    }
		}

		set last_data [lindex $args 0]
		refresh

	    }
	}
    }

    eval [set name]::netgraph $args
}
