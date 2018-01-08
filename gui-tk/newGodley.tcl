proc newOpenGodley {id} {
    if {![ winfo exists ".$id"]} {
        image create cairoSurface $id -surface $id
        toplevel .$id
        wm title .$id "Godley Table:[$id.godleyIcon.table.title]"
        
        label .$id.table -image $id -width 800 -height 200
        bind .$id.table <Configure> "$id.requestRedraw"
        bind .$id.table <Destroy> "$id.delete"

        bind .$id.table <ButtonPress-1> "$id.mouseDown %x %y; focus .$id.table"
        bind .$id.table <ButtonRelease-1> "defaultCursor .$id.table; $id.mouseUp %x %y"
        bind .$id.table <B1-Motion> "motionCursor .$id.table; $id.mouseMove %x %y"

        bind .$id.table <<contextMenu>> "godleyContext $id %x %y %X %Y"
        bind .$id.table <KeyPress> "$id.keyPress %N"
        bind .$id.table <KeyRelease> "$id.keyRelease %N"
        
        menu .$id.context -tearoff 0
        menu .$id.context.import -tearoff 0

        scrollbar .$id.vscroll -orient vertical -command "scrollGodley $id row"
        .$id.vscroll set 0 0.25
        pack .$id.vscroll -side right -fill y
        scrollbar .$id.hscroll -orient horiz -command "scrollGodley $id col"
        pack .$id.hscroll -side bottom -fill x 
        .$id.hscroll set 0 0.25
        pack .$id.table -fill both -expand 1
    }
    wm deiconify .$id
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
    .$id.context add command -label Title -command "setGodleyTitle $id"
    switch [$id.clickType $x $y] {
        background {}
        row0 {
            .$id.context add command -label "Add new stock variable" -command "$id.addStockVar $x"
            .$id.context add cascade -label "Import variable" -menu .$id.context.import
            .$id.context add command -label "Delete stock variable" -command "$id.deleteStockVar $x"
            .$id.context.import delete 0 end
            foreach var [matchingTableColumns $id.godleyIcon [$id.godleyIcon.table.assetClass [$id.colX $x] ]] {
                .$id.context.import add command -label $var -command "importVar $id $var $x"
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
    if [string length [$id.godleyIcon.table.getCell $r $c]] {
        .$id.context add command -label "Cut" -command "godleyCut $id $r $c"
        .$id.context add command -label "Copy" -command "godleyCopy $id $r $c"
    }
    if {![catch {clipboard get -type UTF8_STRING}]} {
        .$id.context add command -label "Paste" -command "godleyPaste $id $r $c"
    }
    tk_popup .$id.context $X $Y
}

proc setGodleyTitle id {
    if {![winfo exists .godleyTitle]} {
        toplevel .godleyTitle
        entry .godleyTitle.entry
        pack .godleyTitle.entry -side top
        buttonBar .godleyTitle "setGodleyTitleOK $id"
    } else {
        wm deiconify .godleyTitle
    }
    .godleyTitle.entry delete 0 end
    .godleyTitle.entry insert 0 [$id.godleyIcon.table.title]
    wm transient .godleyTitle
    focus .godleyTitle.entry
    tkwait visibility .godleyTitle
    grab set .godleyTitle
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
}

proc godleyPaste {id row col} {
    if {![catch {set data [clipboard get -type UTF8_STRING]}]} {
        $id.godleyIcon.setCell $row $col $data
    }
}

