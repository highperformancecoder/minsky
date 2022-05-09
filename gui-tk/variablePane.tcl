proc variablePane {} {
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        wm title .variablePane "Variables"
        image create cairoSurface variableImage -surface minsky.variablePane
        label .variablePane.canvas -image variableImage -width 300 -height 1000
        pack .variablePane.canvas -fill both -expand 1
        bind .variablePane.canvas <Configure> {minsky.variablePane.update %w %h}
        bind .variablePane <KeyPress-Shift_L> {minsky.variablePane.shift 1; .variablePane.canvas configure -cursor $panIcon}
        bind .variablePane <KeyRelease-Shift_L> {minsky.variablePane.shift 0; .variablePane.canvas configure -cursor {}}
        bind .variablePane.canvas <ButtonPress-1> {catch {minsky.variablePane.mouseDown %x %y}}
        bind .variablePane.canvas <ButtonRelease-1> {minsky.variablePane.mouseUp %x %y}
        bind .variablePane.canvas <B1-Motion> {minsky.variablePane.mouseMove %x %y; minsky.variablePane.requestRedraw}
    }
    minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
    raise .variablePane
}

