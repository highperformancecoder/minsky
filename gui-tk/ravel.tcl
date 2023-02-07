# Ravel specific functionality

proc ravelContextItems {} {
    global editorMode
    set editorMode [minsky.canvas.item.editorMode]
            .wiring.context add checkbutton -label "EditorMode" -command minsky.canvas.item.toggleEditorMode -variable editorMode
            .wiring.context add command -label "Export as CSV" -command exportItemAsCSV
            global sortOrder
            set sortOrder [minsky.canvas.item.sortOrder]
            .wiring.context add cascade -label "Set Next Aggregation" -menu .wiring.context.nextAggregationMenu
            .wiring.context add cascade -label "Axis properties" -menu .wiring.context.axisMenu
            if [llength [info commands minsky.canvas.item.lockGroup]] {
                .wiring.context add command -label "Lock specific handles" -command lockSpecificHandles
                .wiring.context add command -label "Unlock" -command {
                    minsky.canvas.item.leaveLockGroup; canvas.requestRedraw
                }
            }
}

proc setupPickDimMenu {} {
    global dimLabelPicked
    if {![winfo exists .wiring.context.pick]} {
        toplevel .wiring.context.pick
        wm title .wiring.context.pick "Pick any two dimensions"
        frame .wiring.context.pick.select
        scrollbar .wiring.context.pick.select.vscroll -orient vertical -command {
            .wiring.context.pick.select.lb yview}
        listbox .wiring.context.pick.select.lb -listvariable dimLabelPicked \
            -selectmode extended -selectforeground blue \
            -width 35 \
            -yscrollcommand {.wiring.context.pick.select.vscroll set} 
        pack .wiring.context.pick.select.lb -fill both  -expand y -side left
        pack .wiring.context.pick.select.vscroll -fill y -expand y -side left
        pack .wiring.context.pick.select
        buttonBar .wiring.context.pick {
            global dimLabelPicked
            set pick {}
            foreach i [.wiring.context.pick.select.lb curselection] {
                lappend pick [lindex $dimLabelPicked $i]
            }
			minsky.canvas.item.setDimLabelsPicked [lindex $pick 0] [lindex $pick 1]
            reset
        }
        button .wiring.context.pick.buttonBar.clear -text "Clear" -command {
            .wiring.context.pick.select.lb selection clear 0 end}
        pack .wiring.context.pick.buttonBar.clear -side left
    } else {
        deiconify .wiring.context.pick
    }
        
    set dimLabelPicked [minsky.canvas.item.dimLabels]
    wm transient .wiring.context.pick
    wm geometry .wiring.context.pick +[winfo pointerx .]+[winfo pointery .]
    ensureWindowVisible .wiring.context.pick
    grab set .wiring.context.pick
}    

proc lockSpecificHandles {} {
    global currentLockHandles

    if {![llength [info commands minsky.canvas.item.lockGroup.allLockHandles]]} {
        minsky.canvas.lockRavelsInSelection
        # reinitialise the canvas item commands
        getItemAt [minsky.canvas.item.x] [minsky.canvas.item.y]
        if {![llength [info commands minsky.canvas.item.lockGroup.allLockHandles]]} return
    }    
    if {[winfo exists .wiring.context.lockHandles]} {destroy .wiring.context.lockHandles}
    toplevel .wiring.context.lockHandles
    frame .wiring.context.lockHandles.grid
    pack .wiring.context.lockHandles.grid

    if {![minsky.canvas.item.lockGroup.handleLockInfo.size]} {
        minsky.canvas.item.lockGroup.setLockHandles [minsky.canvas.item.lockGroup.allLockHandles]
    }    
    
    set col 0
    set ravelNames [minsky.canvas.item.lockGroup.ravelNames]
    foreach r $ravelNames {
        grid [label .wiring.context.lockHandles.grid.name$col -text $r] -row 0 -column $col
        incr col
    }
    global lockHandleAttributes 
    set lockHandleAttributes {"slicer" "orientation" "calipers" "order"}
    set tooltips {
        "slicer includes whether a handle is an output handle or not"
        "orientation includes whether a handle is collapsed or not"
        ""
        "order includes picked slices"
    }
    foreach attribute {"Slicer" "Orientation" "Calipers" "SortOrder"} {
        grid [label .wiring.context.lockHandles.grid.name$col -text $attribute] -row 0 -column $col
        tooltip .wiring.context.lockHandles.grid.name$col [lindex $tooltips [expr $col-[llength $ravelNames]]]
        incr col
    }

    for {set row 1} {$row<=[minsky.canvas.item.lockGroup.handleLockInfo.size]} {incr row} {
        set info [minsky.canvas.item.lockGroup.handleLockInfo.@elem [expr $row-1]]
        set handles [$info.handleNames]
        for {set col 0} {$col<[llength $ravelNames]} {incr col} {
            grid [ttk::combobox .wiring.context.lockHandles.grid.handle${col}_$row -state readony -values [concat "\xA0" [minsky.canvas.item.lockGroup.handleNames $col]]] -row $row -column $col
            .wiring.context.lockHandles.grid.handle${col}_$row set [lindex $handles $col]
        }
        foreach attr $lockHandleAttributes  {
            grid [checkbutton .wiring.context.lockHandles.grid.attr${col}_$row -variable lockHandleAttr(${attr}_$row)] -row $row -column $col
            if [$info.$attr] {
                .wiring.context.lockHandles.grid.attr${col}_$row select
            } else {
                .wiring.context.lockHandles.grid.attr${col}_$row deselect
            }
            incr col
        }
    }

    buttonBar .wiring.context.lockHandles {
        global lockHandleAttributes lockHandleAttr
        for {set row 1} {$row<=[minsky.canvas.item.lockGroup.handleLockInfo.size]} {incr row} {
            set info [minsky.canvas.item.lockGroup.handleLockInfo.@elem [expr $row-1]]
            set handleNames {}
            for {set col 0} {$col<[llength [minsky.canvas.item.lockGroup.ravelNames]]} {incr col} {
                lappend handleNames [.wiring.context.lockHandles.grid.handle${col}_$row get]
            }
            $info.handleNames $handleNames
            foreach attr $lockHandleAttributes {
                $info.$attr $lockHandleAttr(${attr}_$row)
            }
        }
        minsky.canvas.item.lockGroup.validateLockHandleInfo msg
    }
    
    wm transient .wiring.context.lockHandles
    wm geometry .wiring.context.lockHandles +[winfo pointerx .]+[winfo pointery .]
    ensureWindowVisible .wiring.context.lockHandles
    grab set .wiring.context.lockHandles
}

set aggTypes {{"Σ" "sum"} {"Π" "prod"} {"av" "av"} {"σ" "stddev"} {"min" "min"} {"max" "max"}}
menu .wiring.context.nextAggregationMenu
foreach m $aggTypes {
    .wiring.context.nextAggregationMenu add command -label [lindex $m 0] -command "minsky.canvas.item.nextReduction [lindex $m 1]"
}

menu .wiring.context.axisAggregationMenu
foreach m $aggTypes {
    .wiring.context.axisAggregationMenu add command -label [lindex $m 0] -command "minsky.canvas.item.handleSetReduction \[minsky.canvas.item.selectedHandle\] [lindex $m 1]"
}

menu .wiring.context.axisMenu 
.wiring.context.axisMenu add command -label "Description" -command {
    textEntryPopup .wiring.context.axisMenu.desc [minsky.canvas.item.description] {
        minsky.canvas.item.setDescription [.wiring.context.axisMenu.desc.entry get]
    }
}
.wiring.context.axisMenu add command -label "Dimension" -command setDimension
.wiring.context.axisMenu add command -label "Toggle Calipers" -command {
    minsky.canvas.item.toggleDisplayFilterCaliper
    minsky.canvas.item.broadcastStateToLockGroup
    reset
}
.wiring.context.axisMenu add cascade -label "Set Aggregation" -menu .wiring.context.axisAggregationMenu
menu .wiring.context.axisMenu.sort -postcommand populateSortOptions
.wiring.context.axisMenu add cascade -label "Sort" -menu .wiring.context.axisMenu.sort
set sortOrder none

proc populateSortOptions {} {
    set orders {none forward reverse}
    .wiring.context.axisMenu.sort delete 0 end
    foreach order $orders {
        .wiring.context.axisMenu.sort add radiobutton -label $order -command {
            minsky.canvas.item.setSortOrder $sortOrder
            minsky.canvas.item.broadcastStateToLockGroup
            reset
        } -value "$order" -variable sortOrder
    }
    .wiring.context.axisMenu.sort add command -label "forward by value" -command {minsky.canvas.item.sortByValue "forward"; reset}
    .wiring.context.axisMenu.sort add command -label "reverse by value" -command {minsky.canvas.item.sortByValue "reverse"; reset}
}

.wiring.context.axisMenu add command -label "Pick Slices" -command setupPickMenu


proc setDimension {} {
    if {![winfo exists .wiring.context.axisMenu.dim]} {
        toplevel .wiring.context.axisMenu.dim
        wm title .wiring.context.axisMenu.dim "Dimension axis"
        frame .wiring.context.axisMenu.dim.type
        label .wiring.context.axisMenu.dim.type.label -text "type"
        ttk::combobox .wiring.context.axisMenu.dim.type.value -values {string value time} -state readonly -textvariable axisType
        bind .wiring.context.axisMenu.dim.type.value <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.horizontalDimension.type [.wiring.context.axisMenu.dim.type.value get]
            dimFormatPopdown .wiring.context.axisMenu.dim.units.value [.wiring.context.axisMenu.dim.type.value get] {}
        }
        pack .wiring.context.axisMenu.dim.type.label .wiring.context.axisMenu.dim.type.value -side left
        frame .wiring.context.axisMenu.dim.units
        label .wiring.context.axisMenu.dim.units.label -text "units/format"
        tooltip .wiring.context.axisMenu.dim.units.label \
     "Value type: enter a unit string, eg m/s; time type: enter a strftime format string, eg %Y-%m-%d %H:%M:%S, or %Y-Q%Q"
        ttk::combobox .wiring.context.axisMenu.dim.units.value
        dimFormatPopdown .wiring.context.axisMenu.dim.units.value [minsky.canvas.item.dimensionType] {}
        pack .wiring.context.axisMenu.dim.units.label .wiring.context.axisMenu.dim.units.value -side left
        pack .wiring.context.axisMenu.dim.type .wiring.context.axisMenu.dim.units
        buttonBar .wiring.context.axisMenu.dim {
            minsky.canvas.item.setDimension [.wiring.context.axisMenu.dim.type.value get] [.wiring.context.axisMenu.dim.units.value get]
        }
    } else {
        deiconify .wiring.context.axisMenu.dim
    }
    .wiring.context.axisMenu.dim.type.value set [minsky.canvas.item.dimensionType]
    .wiring.context.axisMenu.dim.units.value delete 0 end
    .wiring.context.axisMenu.dim.units.value insert 0 [minsky.canvas.item.dimensionUnitsFormat]
    ensureWindowVisible .wiring.context.axisMenu.dim
    grab set .wiring.context.axisMenu.dim
    wm transient .wiring.context.axisMenu.dim
}

proc setupPickMenu {} {
    global labelPicked pickHandle
    if {![winfo exists .wiring.context.axisMenu.pick]} {
        toplevel .wiring.context.axisMenu.pick
        wm title .wiring.context.axisMenu.pick "Pick slices"
        frame .wiring.context.axisMenu.pick.select
        scrollbar .wiring.context.axisMenu.pick.select.vscroll -orient vertical -command {
            .wiring.context.axisMenu.pick.select.lb yview}
        listbox .wiring.context.axisMenu.pick.select.lb -listvariable labelPicked \
            -selectmode extended -selectforeground blue \
            -width 35 \
            -yscrollcommand {.wiring.context.axisMenu.pick.select.vscroll set} 
        pack .wiring.context.axisMenu.pick.select.lb -fill both  -expand y -side left
        pack .wiring.context.axisMenu.pick.select.vscroll -fill y -expand y -side left
        pack .wiring.context.axisMenu.pick.select
        buttonBar .wiring.context.axisMenu.pick {
            global labelPicked pickHandle
            set pick {}
            foreach i [.wiring.context.axisMenu.pick.select.lb curselection] {
                lappend pick [lindex $labelPicked $i]
            }
            minsky.canvas.item.pickSliceLabels $pickHandle $pick
            minsky.canvas.item.broadcastStateToLockGroup
            reset
        }
        button .wiring.context.axisMenu.pick.buttonBar.all -text "All" -command {
            .wiring.context.axisMenu.pick.select.lb selection set 0 end}
        button .wiring.context.axisMenu.pick.buttonBar.clear -text "Clear" -command {
            .wiring.context.axisMenu.pick.select.lb selection clear 0 end}
        pack .wiring.context.axisMenu.pick.buttonBar.all .wiring.context.axisMenu.pick.buttonBar.clear -side left
    } else {
        deiconify .wiring.context.axisMenu.pick
    }
        
    set labelPicked [minsky.canvas.item.allSliceLabels]
    for {set i 0} {$i<[llength $labelPicked]} {incr i} {
        set idx([lindex $labelPicked $i]) $i
    }
    foreach i [minsky.canvas.item.pickedSliceLabels] {
        .wiring.context.axisMenu.pick.select.lb selection set $idx($i)
    }
    set pickHandle [minsky.canvas.item.selectedHandle]
    wm transient .wiring.context.axisMenu.pick
    wm geometry .wiring.context.axisMenu.pick +[winfo pointerx .]+[winfo pointery .]
    ensureWindowVisible .wiring.context.axisMenu.pick
    grab set .wiring.context.axisMenu.pick
}

