# display widget definitions

proc display {args} {

    if {[lindex $args 0]=="-title"} {
	set title [lindex $args 1]
	set args [lreplace $args 0 1]
    } else {
	set title [lindex $args 1]
    }
	
    set name [string_map {. _ , _ ( _ ) _} "display_[lindex $args 1]"]
    if {![winfo exists .$name]} {
	toplevel .$name 
	wm title .$name $title
	graph .${name}.graph -title $title
	.${name}.graph xaxis configure -title "time"
	.${name}.graph yaxis configure -title ""
	global tcl_platform
	if {$tcl_platform(platform)!="windows" && \
		$tcl_platform(os)!="Darwin"} {
	    Blt_ZoomStack .${name}.graph
	    Blt_Crosshairs .${name}.graph
	    Blt_ActiveLegend .${name}.graph
	    Blt_ClosestPoint .${name}.graph
	}
	frame .$name.buttonbar
	button .${name}.buttonbar.print -text Print -command "
                ${name}::print \[tk_getSaveFile -defaultextension \".ps\" \
		-parent .$name\]"
	button .${name}.buttonbar.clear -text Clear -command ${name}::clear
	pack append .$name.buttonbar .$name.buttonbar.print left \
	    .$name.buttonbar.clear left
	pack append .$name .$name.buttonbar top
	pack append .${name} .${name}.graph top
	
	namespace eval $name {
	    variable time
	    vector [namespace current]::time
	    
	    proc print {{filename display.ps } {options ""}} {
		eval .[namespace tail [namespace current]].graph \
			postscript output $filename $options
	    }

	    proc clear {} {
		[namespace current]::time length 0
		foreach elem [.[namespace tail [namespace current]].graph \
			      element names] {
		    [set elem] length 0
		}
	    }
	}
	
    }
    eval eco_display $args
    return $name
}
