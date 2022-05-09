set checkButtons {}
set varTypes {parameter flow integral stock}

proc variablePane {} {
    global varTypes checkButtons
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        wm title .variablePane "Variables"
        # selection buttons 
        frame .variablePane.buttons
        button .variablePane.buttons.all -text All -command {
            foreach button $checkButtons {$button select}
            minsky.variablePane.selection $varTypes
            minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
        }
        pack .variablePane.buttons.all -side left
        foreach type $varTypes {
            label .variablePane.buttons.${type}Text -text $type
            checkbutton .variablePane.buttons.$type -variable variablePaneButtons($type) -command "variablePaneButtonProc $type"
            lappend checkButtons .variablePane.buttons.$type
            .variablePane.buttons.$type select
            pack .variablePane.buttons.$type .variablePane.buttons.${type}Text  -side left
        }
        pack .variablePane.buttons
        image create cairoSurface variableImage -surface minsky.variablePane
        label .variablePane.canvas -image variableImage -width 300 -height 1000
        pack .variablePane.canvas -fill both -expand 1
        bind .variablePane.canvas <Configure> {minsky.variablePane.update %w %h}
        bind .variablePane <KeyPress-Shift_L> {minsky.variablePane.shift 1; .variablePane.canvas configure -cursor $panIcon}
        bind .variablePane <KeyRelease-Shift_L> {minsky.variablePane.shift 0; .variablePane.canvas configure -cursor {}}
        bind .variablePane.canvas <ButtonPress-1> {catch {minsky.variablePane.mouseDown %x %y}}
        bind .variablePane.canvas <ButtonRelease-1> {minsky.variablePane.mouseUp %x %y}
        bind .variablePane.canvas <B1-Motion> {minsky.variablePane.mouseMove %x %y}
    }
    minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
    raise .variablePane
}

proc variablePaneButtonProc {type} {
    global variablePaneButtons
    if {$variablePaneButtons($type)} {
        minsky.variablePane.select $type
    } else {
        minsky.variablePane.deselect $type
    }
    minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
}
