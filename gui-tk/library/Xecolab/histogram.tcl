
# histogram plot

proc histogram {name args} {
    namespace eval $name {
	if {![llength [info commands init]]} {

	    proc setnbins {x} {
		set name [namespace tail [namespace current]]
		if {$x==[nbins]} return
		nbins [expr int(exp($x/10.0))]
		.$name.nbincontrol configure -label "No. Bins:[nbins]"
		after 100 [namespace code reread]
	    }

	    proc xlogscale {} {
		set name [namespace tail [namespace current]]
		if {[xlogison]} {
		    .$name.xlogscale configure -relief raised
		    xlogison 0
		} else {
		    if { [min] <= 0} {
                        tk_dialog .$name.xlogscalewarn "Nonpositive Data" "Warning: nonpositive data ignored" "" 0 "OK"
		    }
		    .$name.xlogscale configure -relief sunken
		    xlogison 1
		}
		reread
	    }
	    
	    proc ylogscale {} {
		set name [namespace tail [namespace current]]
		if [.$name.graph yaxis cget -logscale] {
		    .$name.ylogscale configure -relief raised
		    .$name.graph yaxis configure -logscale false
		} else {
		    .$name.ylogscale configure -relief sunken
		    .$name.graph yaxis configure -logscale true
		}
	    }

	    # Used to format the x tick labels
	    proc fmttick {name val} {
		return [format "%6.2g" $val]
	    }

	    proc outputdata {} {
		set name [namespace tail [namespace current]]
		set fname [tk_getSaveFile -defaultextension \".dat\" \
			       -parent .$name]
		if {$fname==""} return
		outputdat $fname
	    }

	    proc print {{filename histogram.ps } {options ""}} {
		eval .[namespace tail [namespace current]].graph \
		    postscript output $filename $options
	    }

	    proc histogram {name argl} {

		# grab out title argument and remove title from args list
		if {[lindex $argl 0] == "-title"} {
		    set title [lindex $argl 1]
		    set value [lrange $argl 2 end]
		} else {
		    set title $name
		    set value $argl
		}

		if {![winfo exists .$name]} {
		    toplevel .$name

		    # create the C++ histogram object
		    histogram_data [namespace current]::hist
		    # use_namespace command dumps into global namespace, 
		    # this dumps into current namespace
		    foreach comm [info commands hist.*] {
			regsub {^[^.]*\.} $comm "" procname
			proc $procname {args} "eval $comm \$args"
		    }
		    
		    nbins 99
		    xlogison 0
		    max -1E38
		    min 1E38

		    # log scale controls
		    frame .$name.buttonbar 
		    pack .$name.buttonbar -fill x -side top -in .$name 
		    button .$name.xlogscale -text "x logscale" \
			-command [namespace code "xlogscale"]
		    button .$name.ylogscale -text "y logscale" \
			-command [namespace code "ylogscale"]
		    
		    button .${name}.print -text Print -command [namespace code \
		       "print \[tk_getSaveFile -defaultextension \".ps\" \
			    -parent .$name\]"
		       ]
		    button .${name}.clear -text Clear -command [namespace code clear]
		    pack append .$name.buttonbar .$name.xlogscale left \
			.$name.ylogscale left .${name}.print left .${name}.clear left 

		    # Output histogram data
		    button .$name.outputdata -text "Output Histogram" \
			-command [namespace code "outputdata"]
		    pack append .$name.buttonbar .$name.outputdata left


		    # barchart widget
		    barchart .$name.graph -title $title
		    pack append .$name .$name.graph left
		    global tcl_platform
		    if {$tcl_platform(platform)!="windows" && 
			$tcl_platform(os)!="Darwin"} {
			Blt_ZoomStack .$name.graph	
			Blt_Crosshairs .$name.graph
			Blt_ActiveLegend .$name.graph
			Blt_ClosestPoint .$name.graph
		    }
		    .$name.graph xaxis configure -command [namespace code fmttick]

		    # Scale label for x-axis
		    label .$name.xscale 
		    pack append .$name .$name.xscale bottom
		    
		    vector x([nbins]) 
		    vector y([nbins])
		    .$name.graph element create data -xdata x -ydata y
		    init_BLT_vects $name
		    reread

		    # scroll widget to control number of bins
		    ::scale .$name.nbincontrol -from 0 -to [expr [nbins]+2] \
			-showvalue false\
			-length [expr [.$name.graph cget -height]-7] \
			-command [namespace code "setnbins"]
		    .$name.nbincontrol set 46
		    pack append .$name .$name.nbincontrol right 
		}

		# value is generically a list of list of values
		foreach lv $value {
		    foreach v $lv {
			add_data $v
		    }
		}

	    }
	}
    }
    [set name]::histogram $name $args
    return $name
}
