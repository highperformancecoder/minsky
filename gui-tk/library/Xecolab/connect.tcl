# Connections Plot definitions 

proc connect_plot {args} {

    if {[lindex $args 0]=="-title"} {
	set title [lindex $args 1]
	set args [lreplace $args 0 1]
    } else {
	set title "Connectivity plot"
    }
	
    set name [string_map {. _} "connect_[lindex $args 0]"]

    if {![winfo exists .$name]} {
	toplevel .$name -background aquamarine3 
	wm title .$name $title
	# Controls
	frame .${name}.buttonbar 
	pack append .${name} .${name}.buttonbar top
	
	button .${name}.zoomi -text "Zoom in" -command "
	set ${name}::scale \[ expr \[ set ${name}::scale \] *2 \]
	eval connect_plot $args"

	button .${name}.zoomo -text "Zoom out" -command "
	set ${name}::scale \[expr \[set ${name}::scale\]*.5\]
	eval connect_plot $args"

	button .${name}.print -text Print -command "
		${name}::print \[tk_getSaveFile -defaultextension \".ps\" \
		-parent .$name\]"

	pack append .${name}.buttonbar \
		.${name}.zoomi left  .${name}.zoomo left .${name}.print left
    

	# Canvas widget
	canvas .${name}.graph -height 300  -width 300 -background aquamarine3 \
		-xscrollincrement 1 -yscrollincrement 1
	pack append .${name} .${name}.graph top
	bind .${name}.graph <Configure> "${name}::update_size
          eval eco_connect_plot $args .$name \[set ${name}::scale\]"

	namespace eval $name {variable args}
	set ${name}::args $args
	namespace eval $name {
	# left mouse drags, right mouse zooms
	    variable oldx
	    variable oldy
	    variable scale 4
	    variable connect_offy -1
	    variable args
	    set name [namespace tail [namespace current]]
	    bind .${name}.graph <ButtonPress-1> [namespace code {
		set oldx %x
		set oldy %y 
	    }]
	    bind .${name}.graph <Button1-Motion> [namespace code "
	.${name}.graph xview scroll \[expr \$oldx-%x\] units
	.${name}.graph yview scroll \[expr \$oldy-%y\] units
	set oldx %x
	set oldy %y 
	"]
	bind .${name}.graph <ButtonPress-3> [namespace code "
	set scale \[expr \[set scale\]*2\]
	.${name}.graph xview scroll \
		\[expr round(2*\[.${name}.graph canvasx %x\] \
		- \[.${name}.graph canvasx \
		\[expr \[.${name}.graph cget -width\]/2\]\
		\])\
		\] units
	.${name}.graph yview scroll \
		\[expr round(2*\[.${name}.graph canvasy %y\] \
		- \[.${name}.graph canvasy \
		\[expr \[.${name}.graph cget -height\]/2\]\
		\])\
		\] units
	eval connect_plot $args
	"]
	    proc print {{filename plot.ps } {options ""}} {
		file delete $filename
		eval .[namespace tail [namespace current]].graph \
		    postscript -file $filename $options
	    }

	    proc update_size {} {
		variable connect_offx 
		variable connect_offy
		set name [namespace tail [namespace current]]
		if {$connect_offy==-1} {
		    set connect_offx [expr [winfo width .${name} ] -\
					  [.${name}.graph cget -width]]
		    set connect_offy [expr [winfo height .${name} ] -\
					  [.${name}.graph cget -width]]
		}
		set gwidth [expr [winfo width .${name} ] - $connect_offx]
		set gheight [expr [winfo height .${name}] - $connect_offy]
	    
		.${name}.graph configure -width $gwidth -height $gheight
		.${name}.graph delete all
	    }
		
	}
    } 
    eval eco_connect_plot $args .$name [set ${name}::scale]
    return $name
}

