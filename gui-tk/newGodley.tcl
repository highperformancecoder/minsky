proc newOpenGodley {id} {
    if {![ winfo exists ".$id"]} {
        image create cairoSurface $id -surface $id
        toplevel .$id
        wm title .$id "Godley Table:[$id.godleyIcon.table.title]"
        
        label .$id.table -image $id -width 800 -height 200
        bind .$id.table <Configure> "$id.requestRedraw"
        bind .$id.table <Destroy> "$id.delete"

        bind .$id.table <ButtonPress-1> "mouseDown $id %x %y %X %Y"
        bind .$id.table <ButtonRelease-1> "defaultCursor .$id.table; $id.mouseUp %x %y"
        bind .$id.table <B1-Motion> "motionCursor .$id.table; $id.mouseMoveB1 %x %y"
        bind .$id.table <Motion> "$id.mouseMove %x %y"
        bind .$id.table <Leave> "$id.godleyIcon.update; minsky.canvas.requestRedraw"

        bind .$id.table <<contextMenu>> "godleyContext $id %x %y %X %Y"
        bind .$id.table <KeyPress> "$id.keyPress %N"
        bind .$id.table <KeyRelease> "$id.keyRelease %N"

        global meta meta_menu
        bind .$id.table <$meta-y> "$id.undo -1"
        bind .$id.table <$meta-z> "$id.undo 1"
        bind .$id.table <$meta-x> "cutCopyPaste $id Cut"
        bind .$id.table <$meta-c> "cutCopyPaste $id Copy"
        bind .$id.table <$meta-v> "cutCopyPaste $id Paste"
        
        menu .$id.context -tearoff 0
        menu .$id.context.import -tearoff 0

        scrollbar .$id.vscroll -orient vertical -command "scrollGodley $id row"
        .$id.vscroll set 0 0.25
        pack .$id.vscroll -side right -fill y
        scrollbar .$id.hscroll -orient horiz -command "scrollGodley $id col"
        pack .$id.hscroll -side bottom -fill x 
        .$id.hscroll set 0 0.25
        pack .$id.table -fill both -expand 1

        menu .$id.menubar
        menu .$id.menubar.edit
        .$id configure -menu .$id.menubar
        .$id.menubar.edit add command -label Undo -command "$id.undo 1" -accelerator $meta_menu-Z
        .$id.menubar.edit add command -label Redo -command "$id.undo -1" -accelerator $meta_menu-Y
        .$id.menubar.edit add command -label Title -command "textEntryPopup .godleyTitle {[$id.godleyIcon.table.title]} {setGodleyTitleOK $id}"
        .$id.menubar.edit add command -label Cut -command "cutCopyPaste $id Cut" -accelerator $meta_menu-X
        .$id.menubar.edit add command -label Copy -command "cutCopyPaste $id Copy" -accelerator $meta_menu-C
        .$id.menubar.edit add command -label Paste -command "cutCopyPaste $id Paste" -accelerator $meta_menu-V

        menu .$id.menubar.options
        .$id.menubar.options add checkbutton -label "Show Values" -variable preferences(godleyDisplay) -command setGodleyDisplay
        .$id.menubar.options add checkbutton -label "DR/CR style" -variable preferences(godleyDisplayStyle) -onvalue DRCR -offvalue sign -command setGodleyDisplay
        
        .$id.menubar add cascade -label Edit -menu .$id.menubar.edit -underline 0
        .$id.menubar add cascade -label Options -menu .$id.menubar.options -underline 0
        .$id.menubar add command -label Help -command {help GodleyTable} -underline 0

        global preferences
        $id.displayValues $preferences(godleyDisplay)
        $id.displayStyle $preferences(godleyDisplayStyle)

    }
    wm deiconify .$id
}

proc mouseDown {id x y X Y} {
    if {[$id.clickType $x $y]=="importStock"} {
        set importOptions [matchingTableColumns $id.godleyIcon [$id.godleyIcon.table.assetClass [$id.colX $x] ]]
        if {[llength $importOptions]>0} {
            if {![llength [info commands .$id.import]]} {menu .$id.import}
            .$id.import delete 0 end
            foreach var $importOptions {
                .$id.import add command -label $var -command "importStockVar $id $var $x"
            }
            tk_popup .$id.import $X $Y
        }
    } else {
        $id.mouseDown $x $y
        focus .$id.table
    }
}

proc importStockVar {id var x} {
    set oldVar [$id.godleyIcon.table.getCell 0 [$id.colX $x]]
    if {$oldVar!=""} {
        switch [tk_messageBox -message "Do you wish to overwrite $oldVar?" -type okcancel] {
            ok {$id.importStockVar $var $x}
            cancel {}
        }
    } else {$id.importStockVar $var $x}
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
            setStartVar $cmd $num $id.scrollRowStart [$id.godleyIcon.table.rows]
            set f [expr double([$id.scrollRowStart]-1)/[$id.godleyIcon.table.rows]]
            .$id.vscroll set $f [expr $f+0.25]
        }
        col {setStartVar $cmd $num $id.scrollColStart [$id.godleyIcon.table.cols]
            set f [expr double([$id.scrollColStart]-1)/[$id.godleyIcon.table.cols]]
            .$id.hscroll set $f [expr $f+0.25]
        }
    }
    $id.requestRedraw
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
    .$id.context add command -label Title -command "textEntryPopup .godleyTitle {[$id.godleyIcon.table.title]} {setGodleyTitleOK $id}"
    switch [$id.clickType $x $y] {
        background {}
        row0 {
            .$id.context add command -label "Add new stock variable" -command "$id.addStockVar $x"
            .$id.context add cascade -label "Import variable" -menu .$id.context.import
            .$id.context add command -label "Delete stock variable" -command "$id.deleteStockVar $x"
            .$id.context.import delete 0 end
            foreach var [matchingTableColumns $id.godleyIcon [$id.godleyIcon.table.assetClass [$id.colX $x] ]] {
                .$id.context.import add command -label $var -command "$id.importStockVar $var $x"
            }
        }
        col0 {
            .$id.context add command -label "Add flow" -command "$id.addFlow $y"
            .$id.context add command -label "Delete flow" -command "$id.deleteFlow $y"
        }
        internal {}
    }
    set r [$id.rowY $y]
    set c [$id.colX $x]
    if {$r>=0 && $c>=0} {
        if [string length [$id.godleyIcon.table.getCell $r $c]] {
            .$id.context add command -label "Cut" -command "godleyCut $id $r $c"
            .$id.context add command -label "Copy" -command "godleyCopy $id $r $c"
        }
    }
    if {![catch {clipboard get -type UTF8_STRING}]} {
        .$id.context add command -label "Paste" -command "godleyPaste $id $r $c"
    }
    tk_popup .$id.context $X $Y
}

proc setGodleyTitleOK id {
    $id.godleyIcon.table.title [.godleyTitle.entry get]
    wm title .$id "Godley Table:[$id.godleyIcon.table.title]"
}
    
proc godleyCopy {id row col} {
    clipboard clear
    clipboard append -type UTF8_STRING [$id.godleyIcon.table.getCell $row $col]
}

proc godleyCut {id row col} {
    godleyCopy $id $row $col
    $id.godleyIcon.setCell $row $col {}
    $id.requestRedraw
}

proc godleyPaste {id row col} {
    if {![catch {set data [clipboard get -type UTF8_STRING]}]} {
        $id.godleyIcon.setCell $row $col $data
        $id.requestRedraw
    }
}

proc cutCopyPaste {id cmd} {
    if {[$id.selectedRow]>=0 && [$id.selectedCol]>=0 &&
        ([$id.selectedRow]>0 || [$id.selectedCol]>0)} {
        godley$cmd $id [$id.selectedRow] [$id.selectedCol]
    }
}

proc redrawAllGodleyTables {} {
    foreach c [info commands godleyWindow*.requestRedraw] {$c}
}

# sets each individual Godley table displayValue preference
proc setGodleyDisplay {} {
    global preferences
    foreach c [info commands godleyWindow*.displayValues] {
        $c $preferences(godleyDisplay)
    }
    foreach c [info commands godleyWindow*.displayStyle] {
        $c $preferences(godleyDisplayStyle)
    }
    redrawAllGodleyTables
}
