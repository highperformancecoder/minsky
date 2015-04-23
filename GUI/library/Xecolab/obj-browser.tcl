if {![info exists obj_browse_cnt]} {set obj_browse_cnt 0}

proc read_select {browser} {
    global obj_browse_cnt $browser.lead $browser.struct_cmds $browser.all_cmds

    set name [set $browser.lead][\
	    $browser.vars get [lindex [$browser.vars curselection] 0]]
    if {[regexp -- "->$" $name]} {
	regsub -- "->$" $name .* "name"
	obj_browser $name
    } elseif {[regexp "@" $name]} {
	regsub "\.@.*$" $name "" "name"
	regsub ".*\.@is_" [info commands $name.@is*] "" type
	browse_$type $name
    } elseif {[regexp "::$" $name]} {
        obj_browser * $name
    } else {
	update_browser $browser
    }
}


proc update_browser {browser} {
    set items [$browser.vars curselection]

    foreach item $items {
	set name [$browser.vars get $item]

	if {![regexp -- "->$" $name] && ![regexp -- "::$" $name]} {
	    # strip off previous result, if any
	    regsub "=.*" $name "" name 

	    global obj_browse_cnt $browser.all_cmds $browser.lead

	    # get arguments, if any
	    set args [string trim [$browser.argframe.args get 1.0 end]]
	    $browser.argframe.args delete 1.0 end
	
	    if {[lsearch [set $browser.all_cmds] [set $browser.lead]$name]>=0} {
		if {$args==""} {
		    # this is bizarre - why do we need to call this twice??
		    set result [[set $browser.lead]$name]
		    set result [[set $browser.lead]$name]
		} else {
		    set result [[set $browser.lead]$name $args]
		}

	    } else {
		if {$args==""} {
		    set result [set [set $browser.lead]$name]
		} else {
		    set result [set [set $browser.lead]$name $args]
		}
	    }

	    $browser.vars delete $item
	    $browser.vars insert $item $name=$result
	    $browser.vars itemconfigure $item -foreground red \
		    -selectforeground red
	    $browser.vars selection set $item
	}
    }
}

proc repeat {freq cmd} {
    after $freq "
       $cmd
       repeat $freq {$cmd}
    "
}

proc repeat_plot {obj_browse lead varname} {
    global $lead.$varname.t
    set $lead.$varname.t 0
    set plotwin [string range ${obj_browse}.${varname}_plot 1 end]
    repeat 1000 "
	plot $plotwin $lead.$varname.t \[$lead$varname\]
        incr $lead.$varname.t
    "
}

proc obj_browser {{pat *} {ns ""}} {
    global obj_browse_cnt
    trapabort
    set obj_browse ".obj_browse[incr obj_browse_cnt]"
    toplevel $obj_browse 
    wm title $obj_browse $pat
    frame $obj_browse.argframe
    label $obj_browse.argframe.arglabel -text Args
    text $obj_browse.argframe.args -height 1 -width 15
    button $obj_browse.argframe.repeat -text Repeat -command \
	    "repeat 1000 {update_browser $obj_browse}"
    button $obj_browse.argframe.close -text X -command \
	    "wm withdraw $obj_browse" 
    pack $obj_browse.argframe.arglabel $obj_browse.argframe.args \
	    $obj_browse.argframe.repeat $obj_browse.argframe.close  -side left 
    pack $obj_browse.argframe 
    listbox $obj_browse.vars -width 0 -selectmode extended
    scrollbar $obj_browse.scroll -command "$obj_browse.vars yview"
    $obj_browse.vars configure -yscrollcommand "$obj_browse.scroll set"
    pack $obj_browse.scroll -side left -fill y
    pack $obj_browse.vars -side left -fill both

    menu $obj_browse.rmm
    $obj_browse.rmm add command -label hide -command "
    while {\[llength \[$obj_browse.vars curselection\]\]>0} {
           $obj_browse.vars delete \
                \[lindex \[$obj_browse.vars curselection\] 0\]
        }
        $obj_browse.rmm unpost
    "


    bind $obj_browse.vars <Double-Button-1> "read_select $obj_browse"
    bind $obj_browse.vars <Button-3> "$obj_browse.rmm post %X %Y"
    global $obj_browse.lead $obj_browse.struct_cmds $obj_browse.all_cmds
    global $obj_browse.rmm.items

    # set leadin string to pat minus trailing *
    set $obj_browse.lead [string trimright $pat "*"]

    $obj_browse.rmm add command -label plot -command "
    for {set i 0} {\$i<\[llength \[$obj_browse.vars curselection\]\]} {incr i} {
         regexp {(\[^=\]*)} \[$obj_browse.vars get \[lindex \[$obj_browse.vars curselection\] \$i\]\] {.%s=} varname
        repeat_plot $obj_browse [set $obj_browse.lead] \$varname
       }
        $obj_browse.rmm unpost
    "
      
    # if browsing a bare type, create a "virtual" #value method
    if [llength [info commands [string trimright [set $obj_browse.lead] .]]] {
	proc [set $obj_browse.lead]\#value {args} \
	    "eval [string trimright [set $obj_browse.lead] .] \$args"
    }

    set $obj_browse.all_cmds [uplevel "info commands [set ns]$pat"]
    set $obj_browse.namespaces [namespace eval [set ns] namespace children]
    set $obj_browse.gvars [info vars [set ns]$pat]


    set $obj_browse.structs ""
    set $obj_browse.struct_cmds ""
    set $obj_browse.other_cmds ""
 
    foreach c [lsort [set $obj_browse.all_cmds]] {
	# trim off leadin string
	set c [string range $c [string length [set $obj_browse.lead]] end]
	if [string match {[A-Za-z0-9]*.*} $c] {
	    lappend $obj_browse.struct_cmds $c
	} else {
	    lappend $obj_browse.other_cmds $c
	}
    }
    foreach c [lsort [set $obj_browse.struct_cmds]] {
	set struct_name [string range $c 0 [expr [string first "." $c]-1]]
	if {[lsearch [set $obj_browse.structs] $struct_name]==-1} {
	    lappend $obj_browse.structs  $struct_name
	}
    }
    foreach var [lsort [set $obj_browse.structs]] {
	$obj_browse.vars insert end "$var->"
	$obj_browse.vars itemconfigure end -foreground blue
    }
    foreach var [lsort [set $obj_browse.namespaces]] {
	$obj_browse.vars insert end [set var]::
	$obj_browse.vars itemconfigure end -foreground green \
		-selectforeground green
    }
    foreach var [lsort [set $obj_browse.other_cmds]] {
	$obj_browse.vars insert end $var
	$obj_browse.vars itemconfigure end -foreground red \
		-selectforeground red
    }
    set yview [$obj_browse.vars yview]
    $obj_browse.scroll set [lindex $yview 0] [lindex $yview 1]

    # get current value if simple type
    if [llength [info commands [set $obj_browse.lead]\#value]] {
	$obj_browse.vars selection set 0
	update_browser $obj_browse
    }

}


proc browse_vector {name} {
    global obj_browse_cnt
    set obj_browse ".obj_browse[incr obj_browse_cnt]"
    global $obj_browse.index
    toplevel $obj_browse 
    wm title $obj_browse $name
    scale $obj_browse.sel_idx -from 0 -to [expr [$name.size]-1] -variable $obj_browse.index \
	-resolution 1 -orient horizontal -label index
    button $obj_browse.elem -command "obj_browser \[eval $name.@elem \[set $obj_browse.index\]\].*" \
	-text "Get element"
    pack  $obj_browse.sel_idx $obj_browse.elem
}

proc browse_deque {name} {browse_vector $name}
proc browse_list {name} {browse_vector $name}
proc browse_set {name} {browse_vector $name}

proc getCurrentSelection {name sel_idx} {
    obj_browser [eval $name.@elem [$sel_idx get [lindex [$sel_idx curselection] 0]]].*
}

proc browse_map {name} {
    global obj_browse_cnt
    set obj_browse ".obj_browse[incr obj_browse_cnt]"
    global $obj_browse.index
    toplevel $obj_browse 
    wm title $obj_browse $name
    listbox $obj_browse.sel_idx -width 0 -selectmode extended
    bind $obj_browse.sel_idx <Double-Button-1> \
        "getCurrentSelection $name $obj_browse.sel_idx"

    foreach key [$name.\#keys] {
	$obj_browse.sel_idx insert end $key
    }
    scrollbar $obj_browse.scroll -orient vertical -command "$obj_browse.sel_idx yview"
    pack  $obj_browse.sel_idx $obj_browse.scroll -side left -fill y 
}
