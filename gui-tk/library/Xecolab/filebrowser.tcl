proc browse file {
    global .data.filename .data.pattern
    if [file isdirectory $file] {
	cd $file
	.data.list delete 0 end
	if {![catch {
	    set flist [exec sh -c "ls -a [set .data.pattern] 2>/dev/null"]
	}]} {
	    foreach i $flist {
		.data.list insert end $i
	    }
	}
    } else 	{
	set .data.filename $file
    }
}

proc get_filename {pattern} {
  toplevel .data
  scrollbar .data.scroll -command ".data.list yview"
  listbox .data.list -yscroll ".data.scroll set" -relief raised \
     -width 0 -setgrid yes 
  label .data.label -text "File Selection"
  entry .data.fname -relief raised
  pack .data.label -side top
  frame .data.buttonbar
  button .data.ok -text "OK" -command {browse [.data.fname get]}
  # return blank filename if cancelled
  button .data.cancel -text "Cancel" -command {browse ""}
  pack append .data.buttonbar .data.ok left .data.cancel left
  pack .data.buttonbar -side top
  pack .data.fname -side top
  pack .data.list -side left -fill both -expand yes
  pack .data.scroll -side left -fill y
  bind .data.list <Double-Button-1> {browse [selection get]}
  bind .data.fname <KeyPress-Return> {browse [.data.fname get]}
  # initialize file browser
  global .data.filename .data.pattern
  set .data.pattern $pattern
  browse  .
  set .data.filename ""
  tkwait variable .data.filename
  destroy .data
  return [set .data.filename]
}
