# X-window stuff to go here

Tkinit


# heuristics to discover location of TCL/Tk libraries, as many 
# implementations of TCL/Tk are broken.

#if {$tcl_platform(platform)=="windows"} {
#  set cygp [open {|cygpath -w /} "r"]
#  set cygpath [gets $cygp]
#  set tcl_library [lindex \
#     [glob $cygpath/usr/share/tcl $cygpath/usr/share/tcl\[0-9\]*] \
#     0]
#  set tk_library [lindex \
#     [glob $cygpath/usr/share/tk $cygpath/usr/share/tk\[0-9\]*] \
#     0]
#  close $cygp
#  }
#
#if {! [info exists tcl_library]} {
#    if [info exists env(TCL_LIBRARY)] {set tcl_library $env(TCL_LIBRARY)} {
#	set tcl_library [lindex \
#	[glob ~/usr/lib/tcl ~/usr/lib/tcl\[0-9\]* /usr/lib/tcl \
#		/usr/lib/tcl\[0-9\]* /usr/local/lib/tcl \
#		/usr/local/lib/tcl\[0-9\]* \
#           /usr/share/tcl /usr/share/tcl\[0-9\]*
#	] \
#	0]
#    }
#}
#
#if {! [info exists tk_library]} {
#    if [info exists env(TK_LIBRARY)] {set tk_library $env(TK_LIBRARY)} {
#	set tk_library [lindex \
#	[glob  ~/usr/lib/tk ~/usr/lib/tk\[0-9\]* /usr/lib/tk \
#		/usr/lib/tk\[0-9\]* /usr/local/lib/tk \
#		/usr/local/lib/tk\[0-9\]*\
#           /usr/share/tk /usr/share/tk\[0-9\]*
#	]\
#	0]
#    }
#}
#

# hopefully, we can now rely on TCL/Tk to find its own libraries.
if {![info exists tk_library]} {
  regsub tcl [file tail [info library]] tk tk_library
  set tk_library [file join [file dirname [info library]] $tk_library]
}
source [file join $tk_library tk.tcl]
source [file join $tk_library bgerror.tcl]
#wm geometry . 400x50
wm deiconify .
tk appname [file rootname [file tail $argv(0)]]
wm title . [file rootname [file tail $argv(0)]]

#if {$tcl_platform(platform)=="windows"} {
#  set cygp [open {|cygpath -w /} "r"]
#  set cygpath [gets $cygp]
#  set blt_library [lindex \
#     [glob $env(HOME)/usr/lib/blt2.4 $cygpath/usr/local/lib/blt2.4 \
#     $cygpath/usr/lib/blt2.4] \
#     0]  
#  close $cygp
#  }


if [info exists blt_library] {
    # stuff for BLT
    if [file exists ../library] {
	set blt_library ../library
    }

    lappend auto_path $blt_library

    if { [info commands "namespace"] == "namespace" } {
	if { $tcl_version >= 8.0 } {
	    namespace import blt::*
	} else {
	    catch { import add blt }
	}
	if { $tcl_version >= 8.0 } {
	    namespace import -force blt::tile::*
	} else {
	    import add blt::tile
	}
    } else {
	foreach cmd { button checkbutton radiobutton frame label 
        scrollbar toplevel menubutton listbox } {
	    if { [info command tile${cmd}] == "tile${cmd}" } {
		rename ${cmd} ""
		rename tile${cmd} ${cmd}
	    }
	}    
    }
}


# Top level button bar
frame .buttonbar 

# returns control to main(), which then exits cleanly. 
# exit calls exit(0), which does not clean up properly
button .quit -text quit -command {set running 0; exit_ecolab}

button .run -text run -command {
    .run configure -relief sunken
    .stop configure -relief raised
    set running 1
    simulate}

button .stop -text stop -command {
    global running
    set running 0
    .run configure -relief raised
    .stop configure -relief sunken
}

button .command -text Command -command cli
button .obj_browsw -text "Object Browser" -command obj_browser

button .user1 -text User1 

button .user2 -text User2

#button .user3 -text User3

pack append  .buttonbar .quit  left  .run  left  .stop left 
pack append  .buttonbar .command left .obj_browsw left .user1 left .user2 left  
#.user3 left

pack append . .buttonbar top

proc mem_avail {} {return "[lindex [exec vmstat] 24]KB"}

# Status bar
label .statusbar -text "Not Started Yet"
pack append . .statusbar top

#source $ecolab_library/Xecolab/filebrowser.tcl
source $ecolab_library/Xecolab/obj-browser.tcl
source $ecolab_library/Xecolab/netgraph.tcl
source $ecolab_library/Xecolab/connect.tcl
if [info exists blt_library] {
    source $ecolab_library/Xecolab/display.tcl
    source $ecolab_library/Xecolab/plot.tcl
    source $ecolab_library/Xecolab/histogram.tcl
} else {
    puts stdout "plot, display and histogram not implemented"
}

# memory exhausted dialog box
label .mem_exhausted -text "Memory is Exhausted" -height 5 -relief raised
button .mem_exhausted.ok -text OK -command "place forget .mem_exhausted"
place .mem_exhausted.ok -relx .5 -rely .6 
    


