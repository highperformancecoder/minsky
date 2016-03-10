# plot widget definitions

proc plot {name args} {
    if {![winfo exists .$name]} {
	namespace eval $name {
	    # Used to format the x tick labels
	    proc fmttick {name val} {
		return [format "%6.1f" $val]
	    }

	    proc init {name title} {
		global tcl_platform
		variable xdata
		variable xname
		toplevel .$name
		wm title .$name $title
		graph .$name.graph -title $title 
		frame .$name.buttonbar
		button .${name}.buttonbar.print -text Print -command [namespace code \
		"print \[tk_getSaveFile -defaultextension \".ps\" -parent .$name\]"
		]
		button .${name}.buttonbar.clear -text Clear -command [namespace code clear]

		pack append .$name.buttonbar .$name.buttonbar.print left \
		    .$name.buttonbar.clear left
		pack append .$name .$name.buttonbar top
		pack append .$name .$name.graph top

		if {$tcl_platform(platform)!="windows" && $tcl_platform(os)!="Darwin"} {
		    Blt_ZoomStack .$name.graph
		    Blt_Crosshairs .$name.graph
		    Blt_ActiveLegend .$name.graph
		    Blt_ClosestPoint .$name.graph
		}
		.$name.graph xaxis configure -title $xname
		.$name.graph xaxis configure -command [namespace code fmttick]
		vector xdata
	    }

	    proc clear {} {
		[namespace current]::xdata length 0
		foreach elem [.[namespace tail [namespace current]].graph \
			      element names] {
		    [namespace current]::y[set elem] length 0
		}
	    }

	    proc print {{filename plot.ps } {options ""}} {
		eval .[namespace tail [namespace current]].graph \
			postscript output $filename $options
	    }
	    
	    proc plot {name arglist} {
		variable xdata
		variable lastx 
		variable xname

		# grab out title argument and remove title from args list
		if {[lindex $arglist 0] == "-title"} {
		    set title [lindex $arglist 1]
		    set arglist [lreplace $arglist 0 1]
		} else {
		    set title $name
		}

		if [string_is double [lindex $arglist 0]] {
		    # numerical data passed for x - just call it "x"
		    set xname x
		    set xvalue [lindex $arglist 0]
		} else {
		    # x component is a named variable
		    set xname [lindex $arglist 0]
		    upvar #0 $xname xvalue
		}

		if {![winfo exists .$name]} {
		    set lastx [expr $xvalue-1] 
		    init $name $title
		}

		# don't do unnecessary work
		if {$xvalue==$lastx} return
		set lastx $xvalue
	    
		for {set i 1} {$i<[llength $arglist]} {incr i} {
		    if [string_is double [lindex $arglist $i]] {
			# grab numerical values passed, and name them y1...yn
			set argname y[set i]
			set value [lindex $arglist $i]
		    } else {
			# declare all named arglist variable names as global
			set argname [lindex $arglist $i]
			upvar #0 $argname value
		    }
		
		    if {![llength [vector names [namespace current]::y[set argname]]]} {
			global palette 
			if {[info exists palette]} {
			    set colour \
				    [lindex $palette [expr $i%[llength $palette]]]
			} else {
			    set colour black
			}
			vector y[set argname]
			.$name.graph element create $argname -xdata xdata \
				-ydata y[set argname] -color $colour -pixels 0
		    }
		    y[set argname] append $value
		    xdata append $xvalue
		}
		
	    }

	    
	}
    }
    [set name]::plot $name $args
    return $name
}
