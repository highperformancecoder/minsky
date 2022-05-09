proc variablePane {} {
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        wm title .variablePane "Variables"
        image create cairoSurface variableImage -surface minsky.variablePane
        label .variablePane.canvas -image variableImage -width 300 -height 1000
        pack .variablePane.canvas -fill both -expand 1
        bind .variablePane.canvas <Configure> {minsky.variablePane.update %w %h}
    }
    minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
    raise .variablePane
}

