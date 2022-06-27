proc openGodley {id} {
    if {![ winfo exists ".$id.popup"]} {
        image create cairoSurface $id -surface $id.popup
        toplevel .$id
        wm title .$id "Godley Table:[$id.table.title]"
        $id.deleteCallback "destroy .$id"
        
        frame .$id.controls
        button .$id.controls.run -image runButton -height 25 -width 25 -command runstop -takefocus 0
        button .$id.controls.reset -image resetButton -height 25 -width 25 -command reset -takefocus 0
        button .$id.controls.step -image stepButton -height 25 -width 25  -command {step} -takefocus 0
        bind .$id.controls.step <ButtonPress-1> "set buttonPressed 1; autoRepeatButton .$id.controls.step"
        bind .$id <ButtonRelease-1> {set buttonPressed 0}
        tooltip .$id.controls.run "Run/Stop"
        tooltip .$id.controls.reset "Reset simulation"
        tooltip .$id.controls.step "Step simulation"
        
        label .$id.controls.slowSpeed -text "slow" -takefocus 0
        label .$id.controls.fastSpeed -text "fast" -takefocus 0
        scale .$id.controls.simSpeed -variable delay -command setSimulationDelay -to 0 -from 12 -length 150 -label "Simulation Speed" -orient horizontal -showvalue 0 -takefocus 0

        button .$id.controls.zoomOut -image zoomOutImg -height 24 -width 37  -command "zoomOut $id" -takefocus 0
        tooltip .$id.controls.zoomOut "Zoom Out"
        button .$id.controls.zoomIn -image zoomInImg -height 24 -width 37   -command "zoomIn $id" -takefocus 0
        tooltip .$id.controls.zoomIn "Zoom In"
        button .$id.controls.zoomOrig -image zoomOrigImg -height 24 -width 37 \
            -command "$id.popup.zoomFactor 1; $id.popup.requestRedraw" -takefocus 0
        tooltip .$id.controls.zoomOrig "Reset Zoom"
        pack .$id.controls.run .$id.controls.reset .$id.controls.step .$id.controls.slowSpeed .$id.controls.simSpeed .$id.controls.fastSpeed .$id.controls.zoomOut .$id.controls.zoomIn .$id.controls.zoomOrig -side left
        pack .$id.controls
        
        
        label .$id.table -image $id -width 800 -height 200 -takefocus 1
        bind .$id.table <Configure> "$id.popup.requestRedraw"

        bind .$id.table <ButtonPress-1> "moveAssetClass $id %x %y %X %Y"
        bind .$id.table <ButtonRelease-1> "defaultCursor .$id.table; swapAssetClass $id %x %y"
        bind .$id.table <B1-Motion> "motionCursor .$id.table; $id.popup.mouseMoveB1 %x %y"
        bind .$id.table <Motion> "$id.popup.mouseMove %x %y; $id.popup.requestRedraw"
        bind .$id.table <Leave> "$id.popup.mouseMove -1 -1; $id.update; $id.popup.requestRedraw"
        bind .$id.table <Enter> "$id.popup.adjustWidgets; $id.popup.update; $id.popup.requestRedraw"

        bind .$id.table <<contextMenu>> "godleyContext $id %x %y %X %Y"
        bind .$id.table <Key> "$id.popup.keyPress %N [encoding convertto utf-8 %A] 0 0 0; $id.popup.requestRedraw"
        global meta meta_menu
        bind .$id.table <$meta-y> "$id.popup.undo -1; $id.popup.requestRedraw"
        bind .$id.table <$meta-z> "$id.popup.undo 1; $id.popup.requestRedraw"
        bind .$id <$meta-y> "$id.popup.undo -1; $id.popup.requestRedraw"
        bind .$id <$meta-z> "$id.popup.undo 1; $id.popup.requestRedraw"
        
        bind .$id <$meta-plus> "zoomIn $id"
        bind .$id <$meta-minus> "zoomOut $id"
        bind .$id.table <Key-KP_Add> "zoomIn $id"
        bind .$id.table <Key-KP_Subtract> "zoomOut $id"
        bind .$id <Key-KP_Add> "zoomIn $id"
        bind .$id <Key-KP_Subtract> "zoomOut $id"
        # mouse wheel bindings for X11
        bind .$id.table <Button-4> "zoomIn $id"
        bind .$id.table <Button-5> "zoomOut $id"
        # mouse wheel bindings for pc and aqua
        bind .$id.table <MouseWheel> "if {%D>=0} {zoomIn $id} {zoomOut $id}"

        menu .$id.context -tearoff 0
        menu .$id.context.import -tearoff 0

        frame .$id.vscrollFrame
        scrollbar .$id.vscroll -orient vertical -command "scrollGodley $id row"
        .$id.vscroll set 0 0.25
        pack .$id.vscroll -in .$id.vscrollFrame -anchor n -fill y -side top -expand 1
        ttk::sizegrip .$id.sizegrip
        pack .$id.sizegrip -in .$id.vscrollFrame -anchor s -side bottom
        pack .$id.vscrollFrame -side right -anchor s -fill y 

        scrollbar .$id.hscroll -orient horiz -command "scrollGodley $id col"
        pack .$id.hscroll -side bottom -fill x 
        .$id.hscroll set 0 0.25
        pack .$id.table -fill both -expand 1
        
        menu .$id.menubar -type menubar

        if {[tk windowingsystem] == "aqua"} {
            menu .$id.menubar.apple
            .$id.menubar.apple add command -label "About Minsky" -command aboutMinsky
            .$id.menubar add cascade -menu .$id.menubar.apple
        }
        
        menu .$id.menubar.file
        .$id.menubar.file add command -label "Export" -command "exportGodley $id"
        
        menu .$id.menubar.edit -postcommand "toggleGodleyPaste $id"
        .$id configure -menu .$id.menubar
        .$id.menubar.edit add command -label Undo -command "$id.popup.undo 1" -accelerator $meta_menu-Z
        .$id.menubar.edit add command -label Redo -command "$id.popup.undo -1" -accelerator $meta_menu-Y
        .$id.menubar.edit add command -label Title -command "textEntryPopup .godleyTitle {[$id.table.title]} {setGodleyTitleOK $id}"
        .$id.menubar.edit add command -label Cut -command "$id.popup.cut; $id.popup.requestRedraw" -accelerator $meta_menu-X
        .$id.menubar.edit add command -label Copy -command "$id.popup.copy" -accelerator $meta_menu-C
        .$id.menubar.edit add command -label Paste -command "$id.popup.paste; $id.popup.requestRedraw" -accelerator $meta_menu-V               

        menu .$id.menubar.view
        .$id.menubar.view add command -label "Zoom in" -command "zoomIn $id" -accelerator $meta_menu-+
        .$id.menubar.view add command -label "Zoom out" -command "zoomOut $id" -accelerator $meta_menu--
        .$id.menubar.view add command -label "Reset zoom" -command "$id.popup.zoomFactor 1; $id.popup.requestRedraw"
        
        menu .$id.menubar.options  
        .$id.menubar.options add checkbutton -label "Show Values" -variable preferences(godleyDisplay) -command setGodleyDisplay
        .$id.menubar.options add checkbutton -label "DR/CR style" -variable preferences(godleyDisplayStyle) -onvalue DRCR -offvalue sign -command setGodleyDisplay
        .$id.menubar.options add checkbutton -label "Enable multiple equity columns" -variable preferences(multipleEquities) -command setGodleyDisplay
        
        .$id.menubar add cascade -label File -menu .$id.menubar.file -underline 0
        .$id.menubar add cascade -label Edit -menu .$id.menubar.edit -underline 0
        .$id.menubar add cascade -label View -menu .$id.menubar.view -underline 0
        .$id.menubar add cascade -label Options -menu .$id.menubar.options -underline 0
        .$id.menubar add command -label Help -command {help GodleyTable} -underline 0
       
    }
    wm deiconify .$id
    raise .$id .
}

proc toggleGodleyPaste id {
    if {[getClipboard]==""} {
	.$id.menubar.edit entryconfigure end -state disabled
    } else {
	.$id.menubar.edit entryconfigure end -state normal
    }        
}

proc zoomOut id {
    $id.popup.zoomFactor [expr [$id.popup.zoomFactor]/1.1]
    $id.popup.requestRedraw
}
proc zoomIn id {
    $id.popup.zoomFactor [expr [$id.popup.zoomFactor]*1.1]
    $id.popup.requestRedraw
}

proc mouseDown {id x y X Y} {
    if {[$id.popup.clickTypeZoomed $x $y]=="importStock"} {
        set importOptions [matchingTableColumns $id [$id.table.assetClass [$id.popup.colXZoomed $x] ]]
        if {[llength $importOptions]>0} {
            if {![llength [info commands .$id.import]]} {menu .$id.import}
            .$id.import delete 0 end
            foreach var $importOptions {
                .$id.import add command -label $var -command "importStockVar $id $var $x"
            }
            tk_popup .$id.import $X $Y
        }
    } else {
        $id.popup.mouseDown $x $y
        $id.popup.requestRedraw
        focus .$id.table
    }
}

# warn user when a stock variable column is going to be moved to a different asset class on pressing a column button widget. For ticket 1072.
proc moveAssetClass {id x y X Y} {
	set testStr [$id.popup.moveAssetClass $x $y]
	set c [$id.popup.colXZoomed $x]
    if {$testStr==""} {
		mouseDown $id $x $y $X $Y
	} elseif {$testStr=="Cannot convert stock variable to an equity class"} {
	    tk_messageBox -message $testStr -type ok -parent .$id.table		
    } else {
       switch [tk_messageBox -message $testStr -type yesno -parent .$id.table] {
        yes {mouseDown $id $x $y $X $Y}
        no {mouseDown $id $x $c $X $Y}	  
 	    }
 	 }
}

# warn user when a stock variable column is going to be swapped with a column from a different asset class on mouse click and drag. For ticket 1072.
proc swapAssetClass {id x y} {
	set testStr [$id.popup.swapAssetClass $x $y]
	set c [$id.popup.colXZoomed $x]
    if {$testStr==""} {
		$id.popup.mouseUp $x $y 
	} elseif {$testStr=="Cannot convert stock variable to an equity class"} {
	    tk_messageBox -message $testStr -type ok -parent .$id.table				
    } else {
       switch [tk_messageBox -message $testStr -type yesno -parent .$id.table] {
        yes { $id.popup.mouseUp $x $y }
        no { $id.popup.mouseUp $x $c }
 	 }
    }
    $id.popup.requestRedraw
}

proc importStockVar {id var x} {
    set oldVar [$id.table.getCell 0 [$id.popup.colXZoomed $x]]
    if {$oldVar!=""} {
        switch [tk_messageBox -message "Do you wish to overwrite $oldVar?" -type okcancel] {
            ok {$id.popup.importStockVar $var $x}
            cancel {}
        }
    } else {$id.popup.importStockVar $var $x}
    $id.popup.requestRedraw
}

proc setStartVar {cmd x var max} {
    switch $cmd {
        moveto {
            if {$x<0} return
            $var [expr int($x*$max)]
            if {[$var]<1} {$var 1}
        }
        scroll {
            $var [expr [$var]+$x]
            if {[$var]<1} {$var 1}
            if {[$var]>$max} {$var $max}
        }
    }
}

proc scrollGodley {id rowCol cmd num args} {
    switch $rowCol {
        row {
            setStartVar $cmd $num $id.popup.scrollRowStart [$id.table.rows]
            set f [expr double([$id.popup.scrollRowStart]-1)/[$id.table.rows]]
            .$id.vscroll set $f [expr $f+0.25]
        }
        col {setStartVar $cmd $num $id.popup.scrollColStart [$id.table.cols]
            set f [expr double([$id.popup.scrollColStart]-1)/[$id.table.cols]]
            .$id.hscroll set $f [expr $f+0.25]
        }
    }
    $id.popup.requestRedraw
}

proc motionCursor {w} {
    if {[tk windowingsystem]=="aqua"} {
        $w configure -cursor copyarrow
    } else {
        $w configure -cursor exchange
    }
}

proc defaultCursor {w} {$w configure -cursor {}}

proc godleyContext {id x y X Y} {
    .$id.context delete 0 end
    .$id.context add command -label Help -command {help GodleyTable}
    .$id.context add command -label Title -command "textEntryPopup .godleyTitle {[$id.table.title]} {setGodleyTitleOK $id}"
    switch [$id.popup.clickTypeZoomed $x $y] {
        background {}
        row0 {
            .$id.context add command -label "Add new stock variable" -command "$id.popup.addStockVar $x; $id.popup.requestRedraw"
            .$id.context add cascade -label "Import variable" -menu .$id.context.import
            .$id.context add command -label "Delete stock variable" -command "$id.popup.deleteStockVar $x; $id.popup.requestRedraw"
            .$id.context.import delete 0 end
            foreach var [matchingTableColumns $id [$id.table.assetClass [$id.colXZoomed $x] ]] {
                .$id.context.import add command -label $var -command "$id.popup.importStockVar $var $x; $id.popup.requestRedraw"
            }
        }
        col0 {
            .$id.context add command -label "Add flow" -command "$id.popup.addFlow $y; $id.popup.requestRedraw"
            .$id.context add command -label "Delete flow" -command "$id.popup.deleteFlow $y; $id.popup.requestRedraw"
        }
        internal {}
    }
    set r [$id.popup.rowYZoomed $y]
    set c [$id.popup.colXZoomed $x]
    if {$r>=0 && $c>=0} {
        # if cell $r,$c not already selected, select it
        if {$r!=[$id.popup.selectedRow] || $c!=[$id.popup.selectedCol]} {
            $id.popup.selectedRow $r
            $id.popup.selectedCol $c
            $id.popup.insertIdx 0
            $id.popup.selectIdx 0
        }
        if {[string length [$id.table.getCell $r $c]] && ($r!=1 || $c!=0)} {    # Cannot Cut cell(1,0). For ticket 1064
            .$id.context add command -label "Cut" -command "$id.popup.cut; $id.popup.requestRedraw"    
        }    
        if {[string length [$id.table.getCell $r $c]]} {     
            .$id.context add command -label "Copy" -command "$id.popup.copy"
        }
    }
    if {($r!=1 || $c!=0)} {   # Cannot Paste into cell(1,0). For ticket 1064
        .$id.context add command -label "Paste" -command "$id.popup.paste; $id.popup.requestRedraw"
        if {[getClipboard]==""} {
            .$id.context entryconfigure end -state disabled 
        }
    }
    tk_popup .$id.context $X $Y
}

proc setGodleyTitleOK id {
    $id.table.title [.godleyTitle.entry get]
    wm title .$id "Godley Table:[$id.table.title]"
}
    
# sets each individual Godley table displayValue preference
proc setGodleyDisplay {} {
    global preferences
    setGodleyDisplayValue $preferences(godleyDisplay) $preferences(godleyDisplayStyle)
    multipleEquities $preferences(multipleEquities)
    redrawAllGodleyTables
}

proc exportGodley {id} {
    global workDir type

    set fileTypes [imageFileTypes]
    lappend fileTypes {"LaTeX" .tex TEXT} {"CSV" .csv TEXT}
    set f [tk_getSaveFile -filetypes $fileTypes -initialdir $workDir -typevariable type]  
    if {$f==""} return
    if [renderImage $f $type $id.popup] return
    if {[string match -nocase *.tex "$f"]} {
        eval $id.table.exportToLaTeX {$f}
    } elseif {[string match -nocase *.csv "$f"]} {
        eval $id.table.exportToCSV {$f}
    } else {
        switch $type {
            "LaTeX" {eval $id.table.exportToLaTeX {$f.tex}}
            "CSV" {eval $id.table.exportToCSV {$f.csv}}
        }
    }
}

# make the table window a black hole for tab traversal
rename tk_focusPrev tk_focusPrevOrig
rename tk_focusNext tk_focusNextOrig
proc tk_focusPrev {w} {
    if [regexp "^\.godleyWindow\[0-9\]*\.table" $w] {
        return $w
    } else {
        return [tk_focusPrevOrig $w]
    }
}
proc tk_focusNext {w} {
    if [regexp "^\.godleyWindow\[0-9\]*\.table" $w] {
        return $w
    } else {
        return [tk_focusNextOrig $w]
    }
}
