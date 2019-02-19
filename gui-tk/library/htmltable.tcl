set HMtable(unique) 0
set HMtable(level) 0

;# RCSID: $Header: /home/clif.clif/CVS_ROOT/TclXexample/htmltable.tcl,v 1.6 1999/02/01 05:00:18 clif Exp $
proc HMTBL_insert_FromMap { win text id } {
	global HMtag_map HMinsert_map HMlist_elements HMtable

	upvar #0 HM$win var

	# insert white space (with current font)
	# adding white space can get a bit tricky.  This isn't quite right

	set bad [catch {$win insert $var(S_insert) $HMinsert_map($id) "space $var(font)"}]

	if {!$bad && [lindex $var(fill) end]} {
		set text [string trimleft $text]
	}

	if {[info exists HMinsert_map($id)] && [string match $HMinsert_map($id) "\n"]} {
      	    $win insert $var(S_insert) "\t"
	    for {set i 0} {$i < $HMtable(column)} {incr i} {
      	        $win insert $var(S_insert) "\t"
	    }
	}
}	

################################################################
# proc HMtag_table {win param text}--
#    Process the &lt;TABLE&gt; tag - Called from html_library
#    Creates a new text widget to hold the table
#    Creates HM.WindowName state variables with default values
#    Sets HM.win(divert) to point to the new window so that
#      all text being processed is directed to the table text widget.
#
# Arguments
#   win		The window that will hold everything
#   param	Any parameters in the <TABLE ...> tag
#   text	The name of a variable with associated text
# Results
#   
# 
proc HMtag_table {win param text} {
    global HMtable

    upvar #0 HM$win var
    
    # This tracks the level of tables being rendered - 
    #  When it returns to 0, we can reset the HM_insert_FromMap to the
    #  original value.

    if {$HMtable(level) == 0} {
        rename HMinsert_FromMap HM_ORIG_insert_FromMap
        rename HMTBL_insert_FromMap HMinsert_FromMap
    }
    incr HMtable(level)
    
    # Define a unique name for the text widget 
    #   that will contain this table

    set newTable .txt_$HMtable(unique)

    # Divert output to the new window.

    set var(divert) $newTable
    
    # Make a new global state variable for the new text widget

    upvar #0 HM$newTable newvar
   
    # and initialize it

    HMinit_state $newTable
    set newvar(family) times
    set newvar(size) 12
    set newvar(weight) normal
    set newvar(style) roman
    set newvar(indent) 0
    set newvar(stop) 0
    set newvar(S_insert) end
    set newvar(oldWindow) $win
    
    # Create the new text widget
    update; update idle;
    # puts "PARENT: ($win) [expr [$win cget -width] -2]"
    # puts "PARENT: ($win) [expr [$win cget -insertwidth] -2]"
    # puts "PARENT: ($win) [winfo width $win] "
    # puts "GRID: [wm grid .]"
    set curFont [HMx_font times $newvar(size) medium r]
    # puts "curFont: $curFont"
    text $newTable -width [expr [$win cget -width] -2] -background white \
        -font $curFont -wrap none
    
    # puts "TEXT GRID: [catch {wm grid $newTable} xx]"
    # puts "$xx"
    # puts "TEXT: ($newTable) [winfo width $newTable] "
    # Set the row and column descriptors.

    set HMtable(row) 0
    set HMtable(column) 0
    set HMtable(maxCol) 0
    
    # increment the unique number pointer so that the next table will
    #  get a new text widget name.

    incr HMtable(unique)
}



################################################################
# proc HMtag_/table {win param text}--
#    Process the </TABLE> tag
#    This procedure resets the window to receive text to the master
#    window.
# Arguments
#   
#   table	The window that holds the table
#   param	Any parameters in the </TABLE ...> tag
#   text	The name of a variable with associated text
# 
# Results
#   Maps the new window into the master text window.
#   Sets tab locations in new window
#
proc HMtag_/table {table param text} {
    global HMtable errorInfo

    # Set the var pointer for the table text widget
    upvar #0 HM$table varTbl

    # This tracks the level of tables being rendered - 
    #  When it returns to 0, we can reset the HM_insert_FromMap to the
    #  original value.

    incr HMtable(level) -1
    if {$HMtable(level) == 0} {
        rename HMinsert_FromMap HMTBL_insert_FromMap 
        rename HM_ORIG_insert_FromMap HMinsert_FromMap 
    }
    incr HMtable(level)


    # Set the win and var variables to point to the master text widget,
    #   instead of the table text widget.
    set win $varTbl(oldWindow)
    upvar #0 HM$win var

    # unset var(divert) so that text will no longer be diverted to 
    #  the table window
    unset var(divert)

    # Get the list of tags - to make the loop faster
    set alltags [$table tag names]
    
    set maxFont 10;
    foreach tag $alltags {
        if {[string first "font:" $tag] == 0} {
	    set fontlst [split $tag ":"]
	    set size [lindex $fontlst 2]
	    set family [lindex $fontlst 1]
	    if {($size >= $maxFont) && ([string match $family  "courier"])} {
	        set maxFont $size
		set maxFamily $family
		set txtfont [list [lindex $fontlst 1] [lindex $fontlst 2] [lindex $fontlst 3]]
		}
	}
    }

;# set f [catch {eval HMx_font $txtfont} rslt]
;# puts "F: $f rslt: $rslt"
;#     puts "Plus scaling: [eval HMx_font $txtfont]"

    set txtfont [eval HMx_font $txtfont];
    set maxFont [lindex $txtfont 1]


# puts "txtfont: $txtfont maxFont: $maxFont xfont: $var(xfont) $varTbl(xfont)"
# puts "[$table dump 1.0 20.0]"

    # Reset the height of the table text widget to match the number of
    # rows actually used  and insert it into the master text widget 
    
    set numrow [lindex [split [$table index end] .] 0]

    $table configure -height [expr 1 + $numrow]  -font $txtfont
    $win window create $var(S_insert) -window $table
    update; update idle;

# puts "[$table dump 0.0 end]"
# puts "INDEX: [$table index end]"
    
    set parentPixelWidth [winfo width $win]
    set tablePixelWidth [winfo width $table]
    
    set tableCharWidth [$table cget -width]
    incr tableCharWidth -1

    $table configure -width $tableCharWidth
    update; update idle;
    set tablePixelWidth2 [winfo width $table]
    
    set charPixelWidth [expr $tablePixelWidth - $tablePixelWidth2]

    # Now, I know the width of a char in this font window, calc
    #  how many chars wide to match the parent
    
    set newCharWidth [expr $parentPixelWidth/$charPixelWidth -2]
    $table configure -width $newCharWidth

    # Make a simple little ruler for debugging help.
#    canvas .c932 -height 30 -width $tablePixelWidth -background yellow
#    for {set i 0} {$i < $tablePixelWidth} {incr i 50} {
#        if {($i % 100) == 50} {
#            .c932 create line $i 0 $i 15 -fill green
#	} else {
#            .c932 create line $i 0 $i 30 -fill blue
#	}
#    }
#    $table window create 2.0 -window .c932

#    while {$tablePixelWidth > $parentPixelWidth} {
#        incr charWidth -1
#        # puts "$tablePixelWidth $parentPixelWidth $charWidth"
#	$table configure -width $charWidth
#        set tablePixelWidth [winfo width $table]
#	update; update idle;
#    }


    # Clear out the bogus newlines

#    for {set i 0} {$i <= $HMtable(row)} {incr i} {
#        for {set j 0} {$j < $HMtable(maxCol)} {incr j} {
#
#            # A cell may be blank.  Check for a tag before proceeding
#
#            if {[lsearch $alltags tbl.$i.$j] != -1} {
#                set range [$table tag ranges tbl.$i.$j]
#               # Delete the trailing \n that was added by html_lib.tcl.
#                $table delete "[lindex $range 0] lineend"
#            }
#        }
#    }

    # Find the longest string in each column to set the column width.
    
    # Initialize the sizes for the columns to 0
    
    for {set j 0} {$j < $HMtable(maxCol)} {incr j} {
        set colSize($j) 0;
    }
    

    set curCol 0;
    
    # Go through the dump of the text widget looking for text, and font
    #   and table entry tags.  
    # We track the current font and table cell so that when text comes in
    #  we can calculate the size in the current font, and compare that to 
    #  the previous largest size.

    foreach {id data index} [$table dump 0.0 end] {

        switch $id {
	    "tagon" {
	    	    switch -glob -- $data {
		        "font:*" {
			         set lst [split $data ":"]
           		         set txtfont [list [lindex $lst 1] [lindex $lst 2] [lindex $lst 3]]
                                 set txtfont [eval HMx_font $txtfont];
			         }
		        "tbl.*"  {
			         set lst [split $data "."]
				 set curCol [lindex $lst 2]
			         }
		    }
	     }
	    "text"  {
	    		set line [string trim $data]
                        # Adding a few extra chars seems to take care of
			#  some slack where it appears that htmllib is 
			#  adding some unexpected spaces
			set line "--$line"
                        set len [font measure  $txtfont $line ]

                        if {$len > $colSize($curCol) } {
                            set colSize($curCol) $len
			}
	    }
	}
    }

#    for {set j 0} {$j < $HMtable(maxCol)} {incr j} {
#        puts "$j: $colSize($j)"
#    }



    # Define the tab stops to the maximum
    #   size for each column.
    #
    # The pad value of 8 is to separate the columns
    #
    # If the toplevel window is gridded, divide the
    #   tab position by the grid width
    #
    
    set gridWidth [lindex [wm grid .] 2]
    
    set gridWidth [lindex [wm grid .] 2]
    if {$gridWidth == ""} {set gridWidth 1}

    set tabStops 0;
    set totl 0;

    for {set i 0; set j 1} {$j < $HMtable(maxCol)} {incr j; incr i} {
        set wid [expr ($colSize($i) + 8)/$gridWidth]         
        lappend tabStops [expr $wid + $totl]  
        incr totl $wid
    }

    # and set the tab stops.

    $table configure -tabs $tabStops
    
    # Wipe the first character - a bare (and useless) newline.

    $table delete 1.0
    # foreach {o t th}  [$table dump 0.0 end] { puts [format "%8s %7s .%-50s." $o $th $t]}
    # foreach {o t th}  [$win dump 0.0 3.0] { puts "$o	$t	$th"}

}

################################################################
#  proc HMtag_tr {win param text}--
#    Process the <TR> tag
#    This begins a new row by setting the row and column values.
# Arguments
#   
#   win		The window that holds the table
#   param	Any parameters in the </TR ...> tag
#   text	The name of a variable with associated text
# 
# Results
#   Increments the row position.
#   Sets the column to 0.
# 
 proc HMtag_tr {win param text} {
    global HMtable
    incr HMtable(row)
    set HMtable(column) 0
}


################################################################
# proc HMtag_td {win param text}--
#    Process the <TD> tag
# Arguments
#   
#   win		The window that holds the table
#   param	Any parameters in the <TD ...> tag
#   text	The name of a variable with associated text
# 
# Results
#   Sets a tag for the start of this table entry to be used to 
#   set tab positions
# 
proc HMtag_td {win param text} {
    global HMtable

    upvar $text t
    set t "\t[string trim $t]"

    upvar #0 HM$win var
    set var(Ttab) tbl.$HMtable(row).$HMtable(column)
}

################################################################
# proc HMtag_/td {win param text}--
#    
# Arguments
#
#   win		The window that holds the table
#   param	Any parameters in the </TD ...> tag
#   text	The name of a variable with associated text
# 
# Results
#   Increments the column position.  
#   If there are more columns in this table row than in previous
#      rows, set the max column

proc HMtag_/td {win param text} {
    global HMtable

    upvar $text t
    set t "[string trim $t]"

    incr HMtable(column)
    if {$HMtable(column) > $HMtable(maxCol)} {
	set HMtable(maxCol) $HMtable(column)
    }
}

################################################################
# proc HMlink_callback {win href}--
#    This proc is called by the html_library code to parse a 
#	hypertext reference.
#
# Arguments
#   win		The text window that is used by the html_library to
#  		display the text
#   href	A hypertext reference to use for the next hypertext.
#
# Results
#   This example simply replaces the contents of the display with
#   hardcoded new text.

proc HMlink_callback {win href} {
    global newHTMLtxt

    puts "HMlink_callback was invoked with WIN: $win HREF: $href"

    # Clear the old contents from the window.

    HMreset_win $win

    # Display the new text.

    HMparse_html $newHTMLtxt "HMrender $win"
}

################################################################
# proc HMset_image {win handle src}--
#    Acquire image data, create a Tcl image object, 
#    and return the image handle.
#
# Arguments
#   win		The text window in which the html is rendered.
#   handle	A handle to return to the html library with the image handle
#   src		The description of the image from: <IMG src=XX> 
# 
# Results
#   This example creates a hardcoded image. and then invokes
#   HMgot_image with the handle for that image.

proc HMset_image {win handle src} {
    global logo
    puts "HMset_image was invoked with WIN: $win HANDLE: $handle SRC: $src"

    # In a real application this would parse the src, and load the 
    #  appropriate image data.

    set img [image create photo -data $logo]

    HMgot_image $handle $img

    return ""
}

