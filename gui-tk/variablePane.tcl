proc variablePane {} {
    if {![winfo exists .variablePane]} {
        toplevel .variablePane
        scrollbar .variablePane.scroll  -orient vertical -command variablePaneScroll
        frame .variablePane.images
        pack .variablePane.scroll .variablePane.images -side left -fill y
        bind .variablePane <Enter> updateVariablePane
        bind .variablePane <Leave> {set doUpdateVariablePane 1}
    }
    updateVariablePane
    raise .variablePane
}

set doUpdateVariablePane 1
set variablePaneStart 0
proc updateVariablePane {} {
    global doUpdateVariablePane variablePaneStart
    # <Enter> is triggered too often, so only do it the first time after a <Leave>
    if {!$doUpdateVariablePane} return
    set doUpdateVariablePane 0
    foreach w [winfo children .variablePane.images] {destroy $w}
    foreach i [image names] {
        if [string match variableImage* $i] {
            image delete $i
        }
    }
    minsky.updateVariablePane
    set h 0
    set viewMax 0
    for {set i $variablePaneStart} {$i<[minsky.variablePane.size]} {incr i} {
        set surface [minsky.variablePane.@elem $i]
        image create cairoSurface variableImage$i -surface $surface
        label .variablePane.images.variableImage$i -image variableImage$i -width [$surface.width] -height [$surface.height]
        incr h [expr [$surface.height]+3]
        bind .variablePane.images.variableImage$i <Button-1> $surface.emplace
        if {$h<[winfo screenheight .]} {
            pack .variablePane.images.variableImage$i
            set viewMax $h
        }
    }
    if {$h>0} {
        .variablePane.scroll set [expr double($variablePaneStart)/[minsky.variablePane.size]] [expr double($viewMax)/$h]
    }
}

proc variablePaneScroll {cmd args} {
    global doUpdateVariablePane variablePaneStart
    switch $cmd {
        moveto {set variablePaneStart [expr int([lindex $args 0]*[minsky.variablePane.size])]}
        scroll {incr variablePaneStart [lindex $args 0]}
    }
    if {$variablePaneStart<0} {set variablePaneStart 0}
    set doUpdateVariablePane 1
    updateVariablePane
}
