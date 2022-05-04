proc variablePane {} {
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        bind .variablePane <Enter> updateVariablePane
        bind .variablePane <Leave> {set doUpdateVariablePane 1}
    }
    updateVariablePane
    raise .variablePane
}

set doUpdateVariablePane 1
proc updateVariablePane {} {
    global doUpdateVariablePane
    # <Enter> is triggered too often, so only do it the first time after a <Leave>
    if {!$doUpdateVariablePane} return
    set doUpdateVariablePane 0
    for {set i 0} {$i<[minsky.variablePane.size]} {incr i} {
        pack forget .variablePane.variableImage$i
        destroy .variablePane.variableImage$i
        image delete variableImage$i
    }
    minsky.updateVariablePane
    for {set i 0} {$i<[minsky.variablePane.size]} {incr i} {
        set surface [minsky.variablePane.@elem $i]
        image create cairoSurface variableImage$i -surface $surface
        label .variablePane.variableImage$i -image variableImage$i -width [$surface.width] -height [expr [$surface.height]+3]
        bind .variablePane.variableImage$i <Button-1> $surface.emplace
        pack .variablePane.variableImage$i
    }
}
