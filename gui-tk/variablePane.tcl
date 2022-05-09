proc variablePane {} {
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        image create cairoSurface variableImage -surface minsky.variablePane
        label .variablePane.canvas -image variableImage -width 300 -height 1000
        pack .variablePane.canvas
    }
    minsky.variablePane.update [.variablePane.canvas cget -width] [.variablePane.canvas cget -height]
    minsky.variablePane.requestRedraw
    raise .variablePane
}

