#  @copyright Steve Keen 2012
#  @author Russell Standish
#  This file is part of Minsky.
#
#  Minsky is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Minsky is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
#

set globals(default_rotation) 0

# Wiring canvas


proc createWire {coords} {
    set id [.wiring.canvas create line $coords -tag wires -arrow last -smooth bezier]
    return $id
}

frame .wiring 
frame .wiring.menubar 

# move mode initially
set interactionMode 2 
radiobutton .wiring.menubar.wiringmode -value 1 -variable interactionMode -command setInteractionMode -text wire
radiobutton .wiring.menubar.movemode -value 2 -variable interactionMode -command setInteractionMode -text move
radiobutton .wiring.menubar.panmode -value 3 -variable interactionMode -command setInteractionMode -text pan
radiobutton .wiring.menubar.lassomode -value 4 -variable interactionMode -command setInteractionMode -text lasso

set menubarLine 0
ttk::frame .wiring.menubar.line0

image create photo godleyImg -file $minskyHome/icons/bank.gif
button .wiring.menubar.line0.godley -image godleyImg -height 24 -width 37 \
    -command {addNewGodleyItem [addGodleyTable 10 10]}
tooltip .wiring.menubar.line0.godley "Godley table"

image create photo varImg -file $minskyHome/icons/var.gif
button .wiring.menubar.line0.var -image varImg -height 24 -width 37 \
    -command addVariable
tooltip .wiring.menubar.line0.var "variable"

image create photo constImg -file $minskyHome/icons/const.gif
button .wiring.menubar.line0.const -height 24 -width 37 -image constImg -command {addConstant}
tooltip .wiring.menubar.line0.const "constant"

image create photo integrateImg -file $minskyHome/icons/integrate.gif
button .wiring.menubar.line0.integrate -image integrateImg -command {
    addOperation integrate}
tooltip .wiring.menubar.line0.integrate integrate

#pack .wiring.menubar.movemode .wiring.menubar.wiringmode .wiring.menubar.lassomode .wiring.menubar.panmode -side left

pack .wiring.menubar.line0.godley .wiring.menubar.line0.var .wiring.menubar.line0.const .wiring.menubar.line0.integrate -side left

# create buttons for all available operations (aside from those
# handled especially)
foreach op [availableOperations] {
    if {$op=="numOps"} break
    # ignore some operations
    switch $op {
        "constant" -
        "copy" -
        "integrate"  continue 
    }

    set opTrimmed [regsub {(.*)_$} $op {\1}] 
    # advance to next line in menubar
    if {$op=="data"} {
        incr menubarLine
        ttk::frame .wiring.menubar.line$menubarLine
    }
    if {[tk windowingsystem]=="aqua"} {
        # ticket #187
        image create photo [set op]Img -file $minskyHome/icons/$op.gif
    } else {
        image create photo [set op]Img -width 24 -height 24
        operationIcon [set op]Img $op
    }
    button .wiring.menubar.line$menubarLine.$op -image [set op]Img -command "addOperation $op" -height 24 -width 24
    tooltip .wiring.menubar.line$menubarLine.$op $opTrimmed

    pack .wiring.menubar.line$menubarLine.$op -side left 
    set helpTopics(.wiring.menubar.line$menubarLine.$op) "op:$op"
}

if {[tk windowingsystem]=="aqua"} {
    image create photo switchImg -file $minskyHome/icons/switch.gif
} else {
    image create photo switchImg -width 24 -height 24
    operationIcon switchImg switch
}
button .wiring.menubar.line$menubarLine.switch -image switchImg \
    -height 24 -width 37 -command {placeNewSwitch}
tooltip .wiring.menubar.line$menubarLine.switch "Switch"
pack .wiring.menubar.line$menubarLine.switch -side left 
set helpTopics(.wiring.menubar.line$menubarLine.switch) "switch"


image create photo plotImg -file $minskyHome/icons/plot.gif
button .wiring.menubar.line$menubarLine.plot -image plotImg \
    -height 24 -width 37 -command {newPlot}
tooltip .wiring.menubar.line$menubarLine.plot "Plot"
pack .wiring.menubar.line$menubarLine.plot -side left 

image create photo noteImg -file $minskyHome/icons/note.gif
button .wiring.menubar.line$menubarLine.note -image noteImg \
    -height 24 -width 37 -command {placeNewNote}
tooltip .wiring.menubar.line$menubarLine.note "Note"
pack .wiring.menubar.line$menubarLine.note -side left 

clearAll

# pack menubar lines
for {set i 0} {$i<=$menubarLine} {incr i} {
    pack .wiring.menubar.line$i -side top -anchor w
}
pack .wiring.menubar -fill x

canvas .wiring.canvas -height $canvasHeight -width $canvasWidth -scrollregion {-10000 -10000 10000 10000} \
    -closeenough 2 -yscrollcommand ".vscroll set" -xscrollcommand ".hscroll set"
pack .wiring.canvas -fill both -expand 1

proc get_pointer_x {c} {
  return [expr {[winfo pointerx $c] - [winfo rootx $c]}]
}

proc get_pointer_y {c} {
  return [expr {[winfo pointery $c] - [winfo rooty $c]}]
}

bind . <Key-plus> {zoom 1.1}
bind . <Key-equal> {zoom 1.1}
bind . <Key-minus> {zoom [expr 1.0/1.1]}
# mouse wheel bindings for X11
bind .wiring.canvas <Button-4> {zoom 1.1}
bind .wiring.canvas <Button-5> {zoom [expr 1.0/1.1]}
# mouse wheel bindings for pc and aqua
bind .wiring.canvas <MouseWheel> { if {%D>=0} {zoom 1.1} {zoom [expr 1.0/(1.1)]} }

bind .wiring.canvas <Alt-Button-1> {
    tk_messageBox -message "Mouse coordinates [.wiring.canvas canvasx %x] [.wiring.canvas canvasy %y]"
}

proc zoom {factor} {
    set x0 [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]]
    set y0 [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
    if {$factor>1} {
        .wiring.canvas scale all $x0 $y0 $factor $factor
        minsky.zoom $x0 $y0 $factor
    } else {
        minsky.zoom $x0 $y0 $factor
        .wiring.canvas scale all $x0 $y0 $factor $factor
    }  
    # sliders need to be readjusted, because zooming doesn't do the right thing
    foreach v [variables.visibleVariables] {
        var.get $v
        foreach item [.wiring.canvas find withtag slider$v] {
            set coords [.wiring.canvas coords $item]
            # should be only one of these anyway...
            .wiring.canvas coords $item [var.x] [sliderYCoord [var.y]]
        }
    }
}

.menubar.ops add command -label "Godley Table" -command {addNewGodleyItem [addGodleyTable 10 10]}

.menubar.ops add command -label "Variable" -command "addVariable" 
foreach var [availableOperations] {
    if {$var=="numOps"} break
    .menubar.ops add command -label [regsub {(.*)_$} $var {\1}] -command "addOperation $var"
}

proc clearTempBindings {} {
    bind .wiring.canvas <Motion> {}
    bind .wiring.canvas <Enter> {}
    bind . <Key-Escape> {handleEscapeKey}
    setInteractionMode 2
}

# default command to execute when escape key is pressed
proc handleEscapeKey {} {
    .wiring.context unpost
}
bind . <Key-Escape> {handleEscapeKey}

proc placeNewVar {id} {
    global moveOffsvar$id.x moveOffsvar$id.y
    set moveOffsvar$id.x 0
    set moveOffsvar$id.y 0
    initGroupList
    setInteractionMode 2

    bind .wiring.canvas <Enter> "move var $id %x %y"
    bind .wiring.canvas <Motion> "move var $id %x %y"
    # newly created variables should be locally scoped
    bind .wiring.canvas <Button-1> \
        "clearTempBindings
         checkAddGroup var $id %x %y
         var.get $id
         var.setScope \[var.group\]"
    bind . <Key-Escape> \
        "clearTempBindings
      deleteVariable $id
      .wiring.canvas delete var$id"
}

proc addVariablePostModal {} {
    global globals
    global varInput
    global varType

    set name [string trim $varInput(Name)]
    set varExists [variables.exists $name]
    set id [newVariable $name $varInput(Type)]
    var.get $id
    var.rotation $globals(default_rotation)
    var.init $varInput(Value)
    var.set
    if {!$varExists} {
        value.get [var.valueId]
        setItem var rotation {set varInput(Rotation)}
        setItem var tooltip {set "varInput(Short description)"}
        setItem var detailedText {set "varInput(Detailed description)"}
    }
    closeEditWindow .wiring.initVar

    placeNewVar $id
}

proc addVariable {} {
    global varInput varType initVar_name
    set varType flow
    set varInput(title) "Create Variable"
    addConstantOrVariable
    .wiring.initVar.$initVar_name configure -state enabled
}

proc addConstant {} {
    global varInput varType initVar_name
    set varType constant
    set varInput(title) "Create Constant"
    addConstantOrVariable
    .wiring.initVar.$initVar_name configure -state disabled
}

proc addConstantOrVariable {} {
    global varInput varType
    set varInput(Name) ""
    set varInput(Value) ""
    set varInput(Type) $varType
    deiconifyInitVar
    .wiring.initVar.entry10 configure -values [variables.valueNames]
    ::tk::TabToWindow $varInput(initial_focus);
    tkwait visibility .wiring.initVar
    grab set .wiring.initVar
    wm transient .wiring.initVar
}

proc addOperation {op} {
    global globals constInput
    set id [minsky.addOperation $op]
    op.get $id
    op.rotation $globals(default_rotation)
    op.set
    placeNewOp $id
    if {$op=="constant"} {
	editItem $id op$id
	set constInput(cancelCommand) "cancelPlaceNewOp $id;closeEditWindow .wiring.editConstant"
    }
    return $id
}

# add operation from a keypress
proc addOperationKey {op} {
    global globals constInput
    set id [minsky.addOperation $op]
    op.get $id
    op.rotation $globals(default_rotation)
    op.set
    global moveOffsop$id.x moveOffsop$id.y 
    set moveOffsop$id.x 0
    set moveOffsop$id.y 0

    move op $id [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
    drawOperation $id
    if {$op=="constant"} {
	editItem $id op$id
	set constInput(cancelCommand) "cancelPlaceNewOp $id;closeEditWindow .wiring.editConstant"
    }
    return $id
}

# handle arbitrary text typed into the canvas
set textBuffer ""
proc textInput {char} {
    global textBuffer globals
    #ignore anything unprintable!
    set x [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]]
    set y [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
    if [string is print $char] {
        if {[llength [.wiring.canvas find withtag textBuffer]]==0} {
            .wiring.canvas create text $x $y -tags textBuffer
        }
        append textBuffer $char
        .wiring.canvas itemconfigure textBuffer -text $textBuffer
    } elseif {$char=="\r"} {
        .wiring.canvas delete textBuffer
        if {[lsearch [availableOperations] $textBuffer]>-1} {
            addOperationKey $textBuffer
        } elseif {![string match "\[%#\]*" $textBuffer]} {
            # if no space in text, add a variable of that name
            set id [newVariable $textBuffer "flow"]
            var.get $id
            var.rotation $globals(default_rotation)
            var.moveTo $x $y
            initGroupList
            setInteractionMode 2
            newVar $id
        } else {
            set id [minsky.newNote]
            note.get $id
            # trim off leading comment character
            note.detailedText [string range $textBuffer 1 end]
            note.moveTo [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]]\
                [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
            newNote $id
        }
        # TODO add arbitrary comment boxes
        set textBuffer ""
    }
}

# operation add shortcuts
bind . <Key-plus> {addOperationKey add}
bind . <Key-KP_Add> {addOperationKey add}
bind . <Key-minus> {addOperationKey subtract}
bind . <Key-KP_Subtract> {addOperationKey subtract}
bind . <Key-asterisk> {addOperationKey multiply}
bind . <Key-KP_Multiply> {addOperationKey multiply}
bind . <Key-slash> {addOperationKey divide}
bind . <Key-KP_Divide> {addOperationKey divide}
bind . <Key-asciicircum> {addOperationKey pow}
#bind . <Key-backslash> {addOperationKey sqrt}
bind . <Key-ampersand> {addOperationKey integrate}
bind . <Key-equal> {addNewGodleyItemKey}
bind . <Key-at> {newPlotItem [plots.nextPlotID] [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}

bind . <Key> {textInput %A}

bind . <Key-Delete> {deleteKey}
bind . <Key-BackSpace> {deleteKey}

bind . <KeyPress-Shift_L> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_L> {.wiring.canvas configure -cursor arrow}
bind . <KeyPress-Shift_R> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_R> {.wiring.canvas configure -cursor arrow}

# handle processing when delete or backspace is pressed
proc deleteKey {} {
    global textBuffer 
    if {[string length $textBuffer]>0} {
        set textBuffer [string range $textBuffer 0 end-1]
        .wiring.canvas itemconfigure textBuffer -text $textBuffer
    } else {
        set tags [.wiring.canvas gettags  [.wiring.canvas find withtag current]]
        set re {([a-zA-Z]+)([0-9]+)}
        if [regexp $re [lsearch -regexp -inline $tags $re] tag item id] {
            deleteItem $id $tag
        }
    }
}

proc placeNewOp {opid} {
    global moveOffsop$opid.x moveOffsop$opid.y
    set moveOffsop$opid.x 0
    set moveOffsop$opid.y 0
    initGroupList
    setInteractionMode 2

    drawOperation $opid
    bind .wiring.canvas <Enter> "move op $opid %x %y"
    bind .wiring.canvas <Motion> "move op $opid %x %y"
    bind .wiring.canvas <Button-1> \
        "clearTempBindings
         checkAddGroup op $opid %x %y"
    bind . <Key-Escape> \
        "clearTempBindings
      deleteOperation $opid
      .wiring.canvas delete op$opid"
   
}

proc cancelPlaceNewOp {id} {
    bind .wiring.canvas <Motion> {}
    bind .wiring.canvas <Enter> {}
    bind .wiring.canvas <Button> {}
    .wiring.canvas delete op$id
    deleteOperation $id
    updateCanvas
}

proc redraw {item} {
    .wiring.canvas coords $item [.wiring.canvas coords $item]
    .wiring.canvas delete handles
    foreach w [.wiring.canvas find withtag wires] {
        set tags [.wiring.canvas gettags $w]
        set tag [lindex $tags [lsearch -regexp $tags {wire[0-9]+}]]
        set id [string range $tag 4 end]
        if [string length $id] {
            wire.get $id
            .wiring.canvas coords $w [wire.coords]
        }
    }
}

set itemFocused 0

proc deleteTooltipIfLeft {item id} {
    set x [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]] 
    set y [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
    $item.get $id
    if {[$item.clickType $x $y]=="outside"} {
        # throttle deletion
        after 100 {.wiring.canvas delete tooltip}
    }
}


set inItemEnterLeave 0
proc itemEnterLeave {item id tag enter} {
    scopedDisableEventProcessing
    
    global itemFocused inItemEnterLeave
    if {$inItemEnterLeave} return
    set inItemEnterLeave 1

    $item.get $id
    # preserve dirty flag
    set edited [edited] 
    $item.mouseFocus $enter
    if {!$edited} resetEdited
    resetNotNeeded
    redraw $tag
    set x [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]] 
    set y [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
    if {$enter} {
        if {!$itemFocused && [llength [.wiring.canvas find withtag tooltip]]==0} {
            .wiring.canvas create text [expr [$item.x]+20] [expr [$item.y]-20] -tags tooltip -text  [$item.tooltip]
            .wiring.canvas bind tooltip <Enter> {}
            # delete ourself if we've left the icon
            .wiring.canvas bind tooltip <Leave> "deleteTooltipIfLeft $item $id"
        }
    } else {
        deleteTooltipIfLeft $item $id
    }
    set itemFocused $enter
    set inItemEnterLeave 0
}

proc drawOperation {id} {
    op.get $id

    if {[lsearch -exact [image name] opImage$id]!=-1} {
        image delete opImage$id
    }
    image create photo opImage$id -width 200 -height 200

    .wiring.canvas delete op$id
    .wiring.canvas create operation [op.x] [op.y] -id $id -image opImage$id -tags "op$id operations" 
#    .wiring.canvas create rectangle [.wiring.canvas bbox op$id] -tags op$id

    setM1Binding op $id op$id
    op.get $id
    .wiring.canvas bind op$id <<middleMouse>> \
        "wires::startConnect [lindex [op.ports] 0] op$id %x %y"
    .wiring.canvas bind op$id <<middleMouse-Motion>> \
        "wires::extendConnect [lindex [op.ports] 0] op$id %x %y"
    .wiring.canvas bind op$id <<middleMouse-ButtonRelease>> \
        "wires::finishConnect op$id %x %y"
    .wiring.canvas bind op$id  <Double-Button-1> "doubleClick op$id %X %Y"
    .wiring.canvas bind op$id <Enter> "itemEnterLeave op $id op$id 1"
    .wiring.canvas bind op$id <Leave> "itemEnterLeave op $id op$id 0"
}

proc updateItemPos {item id} {
    global globals
    catch {
        # ignore errors that may occur if the object vanishes before now
        $item.get $id
        eval .wiring.canvas coords $item$id [$item.x] [$item.y]
        foreach p [$item.ports]  {
            adjustWire $p
        }
    }
    unset globals(updateItemPositionSubmitted$item$id)
    resetNotNeeded
    doPushHistory 1
}    

proc submitUpdateItemPos {item id} {
    global globals
    if {!
        ([info exists globals(updateItemPositionSubmitted$item$id)] &&
         [set globals(updateItemPositionSubmitted$item$id)])} {
        # only submitted if no update already scheduled
        set globals(updateItemPositionSubmitted$item$id) 1
        after idle updateItemPos $item $id
    }
}

# moveSet is used to determine the offset of where the mouse was
# clicked to the anchor point of the item, so that clicking on an item
# doesn't cause it to jump
proc moveSet {item id x y} {
    $item.get $id
    set x [.wiring.canvas canvasx $x]
    set y [.wiring.canvas canvasy $y]
    global moveOffs$item$id.x moveOffs$item$id.y
    set moveOffs$item$id.x [expr $x-[$item.x]]
    set moveOffs$item$id.y [expr $y-[$item.y]]
    if {"$item"=="group"} {
        initGroupList $id
    } {
        initGroupList
    }
}

proc move {item id x y} {
    doPushHistory 0
    $item.get $id
   global moveOffs$item$id.x moveOffs$item$id.y
# ticket #220: Windows 8 does not always generate mousedown events
# before sending mouse move events. This little bit of merde fakes a
# mousedown event if it hasn't been received yet. This at least gets
# rid of the error message, but the "acceleration" will probably still
# be there.
    if {![info exists moveOffs$item$id.x] || ![info exists moveOffs$item$id.y]} {
        moveSet $item $id $x $y
    }

    set x [expr $x-[set moveOffs$item$id.x]]
    set y [expr $y-[set moveOffs$item$id.y]]
    $item.moveTo [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]
    $item.zoomFactor [localZoomFactor $item $id [$item.x] [$item.y]]
    $item.set $id
    submitUpdateItemPos $item $id
    switch $item {
        "var" {
            foreach item [.wiring.canvas find withtag slider$id] {
                set coords [.wiring.canvas coords $item]
                # should be only one of these anyway...
                .wiring.canvas coords $item [.wiring.canvas canvasx $x] \
                    [sliderYCoord [.wiring.canvas canvasy $y]]
            }
        }
    }
    resetNotNeeded
}

# create a new canvas item for var id
proc newVar {id} {
    global globals
    var.get $id
    if {[lsearch -exact [image name] varImage$id]!=-1} {
        image delete varImage$id
    }
    image create photo varImage$id -width 200 -height 50
    
    .wiring.canvas delete var$id
    set itemId [.wiring.canvas create variable [var.x] [var.y] -image varImage$id -id $id -tags "variables var$id"]
    # wire drawing. Can only start from an output port
    .wiring.canvas bind var$id <<middleMouse>> \
        "wires::startConnect [var.outPort] var$id %x %y"
    .wiring.canvas bind var$id <<middleMouse-Motion>> \
        "wires::extendConnect [var.outPort] var$id %x %y"
    .wiring.canvas bind var$id <<middleMouse-ButtonRelease>> "wires::finishConnect var$id %x %y"

    .wiring.canvas bind var$id <Double-Button-1> "doubleClick var$id %X %Y"
    .wiring.canvas bind var$id <Enter> "itemEnterLeave var $id var$id 1"
    .wiring.canvas bind var$id <Leave> "itemEnterLeave var $id var$id 0"
}

proc addNewGodleyItem {id} {
    global moveOffsgodley$id.x moveOffsgodley$id.y
    set moveOffsgodley$id.x 0
    set moveOffsgodley$id.y 0
    setInteractionMode 2

    
    newGodleyItem $id
  
    # event bindings for initial placement
    bind .wiring.canvas <Enter> "move godley $id %x %y"
    bind .wiring.canvas <Motion> "move godley $id %x %y"
    bind .wiring.canvas <Button-1> clearTempBindings
    bind . <Key-Escape> "clearTempBindings
       deleteGodleyTable $id
       .wiring.canvas delete godley$id"

}

proc addNewGodleyItemKey {} {
    set id [addGodleyTable 10 10]
    global moveOffsgodley$id.x moveOffsgodley$id.y
    set moveOffsgodley$id.x 0
    set moveOffsgodley$id.y 0
    setInteractionMode 2

    newGodleyItem $id
  
    move godley $id [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

# global godley icon resource
#godley.iconResource $minskyHome/icons/bank.svg
#godley.svgRenderer.setResource $minskyHome/icons/bank.svg
setGodleyIconResource $minskyHome/icons/bank.svg

proc godleyToolTipText {id x y} {
    godley.get $id
    set v [godley.select [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
    if {$v>=0} {
        var.get $v
        set text [var.name]
    } else {
        set text [godley.tooltip]
        if {$text==""} {
            set text [godley.table.title]
        }
    }
    return $text
}

proc godleyToolTip {id x y} {
    if {[llength [.wiring.canvas find withtag tooltip]]==0} {
        .wiring.canvas create text [.wiring.canvas canvasx [expr $x+20]] [.wiring.canvas canvasy [expr $y-20]] -tags "tooltip tooltiptext" -anchor w -text "[godleyToolTipText $id $x $y]"
        .wiring.canvas create rectangle [.wiring.canvas bbox tooltiptext] -fill white -tags "tooltip tooltipBG"
        .wiring.canvas raise tooltiptext
    }
}

proc changeToolTip {id x y} {
    .wiring.canvas itemconfigure tooltiptext -text "[godleyToolTipText $id $x $y]"
    .wiring.canvas coords tooltiptext [.wiring.canvas canvasx [expr $x+20]] [.wiring.canvas canvasy [expr $y-20]]
    .wiring.canvas coords tooltipBG [.wiring.canvas bbox tooltiptext]
}



proc newGodleyItem {id} {
    global minskyHome

    godley.get $id

    .wiring.canvas create godley [godley.x] [godley.y] -id $id -tags "godleys godley$id"
    .wiring.canvas lower godley$id

    setM1Binding godley $id godley$id
    .wiring.canvas bind godley$id <<middleMouse>> \
        "wires::startConnect \[closestOutPort %x %y \] godley$id %x %y"
    .wiring.canvas bind godley$id <<middleMouse-Motion>> \
        "wires::extendConnect \[closestOutPort %x %y \] godley$id %x %y"
    .wiring.canvas bind godley$id <<middleMouse-ButtonRelease>> \
        "wires::finishConnect godley$id %x %y"
    .wiring.canvas bind godley$id  <Double-Button-1> "doubleMouseGodley $id %x %y"
    .wiring.canvas bind godley$id <Enter> "godleyToolTip $id %x %y; itemEnterLeave godley $id godley$id 1"
    .wiring.canvas bind godley$id <Leave> "itemEnterLeave godley $id godley$id 0"
    .wiring.canvas bind godley$id <Motion> "changeToolTip $id %x %y"
}

proc rightMouseGodley {id x y X Y} {
    godley.get $id
    set var [godley.select [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
    if {$var==-1} {
        contextMenu godley$id $X $Y
    } else {
        .wiring.context delete 0 end
        .wiring.context add command -label "Edit" -command "editItem $var var"
        var.get $var
        .wiring.context add command -label "Copy" -command "
           copyVar $var
           var.rotation 0
           var.set
        "
        .wiring.context post $X $Y

    }
}

proc doubleMouseGodley {id x y} {
    godley.get $id
    set var [godley.select [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
    if {$var==-1} {
        openGodley $id
    } else {
        editItem $var var
    }
}

proc updateGodleyItem {id} {
    .wiring.canvas delete godley$id
    newGodleyItem $id
}

proc newWire {wire wireid} {
    wire.get $wireid
    .wiring.canvas addtag wire$wireid withtag $wire 
    .wiring.canvas addtag groupitems[wire.group] withtag $wire 
    .wiring.canvas bind $wire <Enter> "decorateWire $wireid; set itemFocused 1"
    .wiring.canvas bind $wire <Leave> "set itemFocused 0"
    # mouse-1 clicking on wire starts wiring from the from port
    .wiring.canvas bind $wire <Button-1> "set clicked 1; wires::startConnect [wire.from] $wire %x %y"
    .wiring.canvas bind $wire <B1-Motion> "wires::extendConnect [wire.from] $wire %x %y"
    .wiring.canvas bind $wire <B1-ButtonRelease> "set clicked 0; wires::finishConnect $wire %x %y"
}

proc updateCoords {wire handle pos x y} {
#    assert {[llength [.wiring.canvas find withtag wire$wire]]==1}
    set coords [.wiring.canvas coords wire$wire]
    set x0 [lindex $coords $pos]
    set y0 [lindex $coords [expr $pos+1]]
    set x [.wiring.canvas canvasx $x]
    set y [.wiring.canvas canvasy $y]
    .wiring.canvas move $handle [expr $x-$x0] [expr $y-$y0]
    lset coords $pos $x
    lset coords [expr $pos+1] $y
    .wiring.canvas coords wire$wire $coords
    wire.get $wire
    wire.coords $coords
    wire.set
}

proc insertCoords {wire handle pos x y} {
    global handles
#    assert {[llength [.wiring.canvas find withtag wire$wire]]==1}
    if {![info exists handles($handle)]} {
        set handles($handle) 1
        # add current handle coordinates to the wire shape
        set coords [.wiring.canvas coords wire$wire]
        set coords [linsert $coords $pos [.wiring.canvas canvasx $x] \
                    [.wiring.canvas canvasy $y]]
        .wiring.canvas coords wire$wire $coords
    }
    updateCoords $wire $handle $pos $x $y
}

proc createHandle {w x y} {
    return [
# attach wire information, allowing a wire context menu to be posted on right mouse click
        .wiring.canvas create oval \
        [expr $x-3] [expr $y-3] [expr $x+3] [expr $y+3] \
        -fill blue  -tag "wires wire$w handles"
    ]
}

proc deleteHandle {wire handle pos} {
    .wiring.canvas delete $handle
    set coords [lreplace [.wiring.canvas coords wire$wire] $pos [expr $pos+1]]
    .wiring.canvas coords wire$wire $coords        
    wire.get $wire
    wire.coords $coords
    wire.set
    decorateWire $wire
}
    

proc decorateWire {wire} {
#    global TCLwireid
#    set wireid $TCLwireid($wire)
    .wiring.canvas delete handles
    set coords [.wiring.canvas coords wire$wire]
    for {set i 0} {$i<[llength $coords]-2} {incr i 2} {
        if {$i>0} {
            set h [
                   createHandle $wire \
                       [lindex $coords $i] [lindex $coords [expr $i+1]]
                  ]
            .wiring.canvas bind $h <B1-Motion> \
                "updateCoords $wire $h $i %x %y"
            .wiring.canvas bind $h <Double-Button-1> \
                "deleteHandle $wire $h $i"
            }
        # create a handle in between
        set h [
            createHandle $wire [
                expr ([lindex $coords $i]+[lindex $coords [expr $i+2]])/2
            ] [
                expr ([lindex $coords [expr $i+1]]+\
                    [lindex $coords [expr $i+3]])/2]
        ]
        .wiring.canvas bind $h <B1-Motion> \
            "insertCoords $wire $h [expr $i+2] %x %y" 
        .wiring.canvas bind $h <Double-Button-1> \
            "deleteHandle $wire $h [expr $i+2]"
    }
}

namespace eval wires {
    proc startConnect {portId id x y} {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        namespace children
        if {![namespace exists $id]} {
            namespace eval $id {
                variable wire
                variable fromPort
                variable x0
                variable y0 
               
                proc extendConnect {x y} {
                    variable wire
                    variable x0
                    variable y0 
                    .wiring.canvas coords $wire $x0 $y0 $x $y
                }
                
                proc finishConnect {x y} {
                    set x [.wiring.canvas canvasx $x]
                    set y [.wiring.canvas canvasy $y]
                    variable wire
                    variable fromPort
                    variable x0
                    variable y0 
                    set portId [closestInPort $x $y]
                    if {$portId>=0} {
                        port.get $portId
                        eval .wiring.canvas coords $wire $x0 $y0 [port.x] [port.y]
                        set wireid [addWire $fromPort $portId \
                                        [.wiring.canvas coords $wire]]
                        if {$wireid == -1} {
                            # wire is invalid
                            .wiring.canvas delete $wire
                        } else {
                            newWire $wire $wireid
                        }
                    } else {
                        .wiring.canvas delete $wire
                    }
                    namespace delete [namespace current]
                }

            }

            set [set id]::wire [createWire "$x $y $x $y"]
#            set port [ports.@elem $portId]
            set [set id]::fromPort $portId
            port.get $portId
            set [set id]::x0 [port.x]
            set [set id]::y0 [port.y]
        }
#        [set id]::startConnect $x $y
    }        

    proc extendConnect {portId id x y} {
        if [namespace exists $id] {
            set x [.wiring.canvas canvasx $x]
            set y [.wiring.canvas canvasy $y]
            [set id]::extendConnect $x $y
        }
    }

    proc finishConnect {id x y} {[set id]::finishConnect $x $y}
}

# adjust the begin or end of line when a port moves
proc adjustWire {portId} {
    foreach w [wiresAttachedToPort $portId] {
        .wiring.canvas delete handles
        wire.get $w
        if [wire.visible] {
            # ensure wire exists on canvas
            if {[.wiring.canvas type wire$w]==""} {
                newWire [createWire [wire.coords]] $w
            }
            eval .wiring.canvas coords wire$w [wire.coords]
        }
   }
}

proc straightenWire {id} {
    wire.get $id
    port.get [wire.from]
    set fx [port.x]
    set fy [port.y]
    port.get [wire.to]
    wire.coords "$fx $fy [port.x] [port.y]"
    wire.set
    eval .wiring.canvas coords wire$id [wire.coords]
}

# a closestOutPort that takes into account panning
proc closestOutPort {x y} {
    return [minsky.closestOutPort [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
}

# track button state
set clicked 0
proc unbindOnRelease {tag} {
    global clicked
    set clicked 0
    .wiring.canvas bind $tag <B1-Motion> ""
    .wiring.canvas bind $tag <B1-ButtonRelease> ""
}

# called when clicking on an item 
proc onClick {item id tag x y} {
    global clicked
    set clicked 1
    $item.get $id
    switch [$item.clickType [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]] {
        "onPort" {
            wires::startConnect [closestOutPort $x $y] $tag $x $y
            .wiring.canvas bind $tag <B1-Motion> \
                "wires::extendConnect \[closestOutPort %x %y\] $tag %x %y"
            .wiring.canvas bind $tag <B1-ButtonRelease>  \
                "wires::finishConnect $tag %x %y; unbindOnRelease $tag"
        }
        "onItem" {
            moveSet $item $id $x $y
             .wiring.canvas bind $tag <B1-Motion> "move $item $id %x %y"
            .wiring.canvas bind $tag <B1-ButtonRelease> "move $item $id %x %y; checkAddGroup $item $id %x %y; unbindOnRelease $tag"
        }
        "outside" {
            .wiring.canvas bind $tag <B1-Motion> "lasso %x %y"
            .wiring.canvas bind $tag <B1-ButtonRelease> "lassoEnd %x %y; unbindOnRelease $tag"
        }
    }
}

proc setM1Binding {item id tag} {
    global interactionMode
    switch $interactionMode {
        1 {
            $item.get $id
            # wiring mode
            .wiring.canvas bind $tag <Button-1> \
                "wires::startConnect \[closestOutPort %x %y\] $tag %x %y"
            .wiring.canvas bind $tag <B1-Motion> \
                "wires::extendConnect \[closestOutPort %x %y\] $tag %x %y"
            .wiring.canvas bind $tag <B1-ButtonRelease>  \
                "wires::finishConnect $tag %x %y"
        }
        2 { 
            .wiring.canvas bind $tag <Button-1> "onClick $item $id $tag %x %y"
        }
        default { 
            # pan mode
            .wiring.canvas bind $tag <Button-1> ""
            .wiring.canvas bind $tag <B1-Motion> ""
            .wiring.canvas bind $tag <B1-ButtonRelease> ""
        }
    }
}

proc setInteractionMode {args} {
    global interactionMode clicked
    if [llength $args] {set interactionMode [lindex $args 0]}

    bind .wiring.canvas <Button-1> ""
    bind .wiring.canvas <B1-Motion> ""
    bind .wiring.canvas <B1-ButtonRelease> ""

    # remove any insertion bindings in place
    bind .wiring.canvas <Motion> {}
    bind .wiring.canvas <Enter> {}
    
    switch -glob $interactionMode {
        3 {
            bind .wiring.canvas <Button-1> {.wiring.canvas scan mark %x %y}
            bind .wiring.canvas <B1-Motion> {.wiring.canvas scan dragto %x %y 1}
        } 
        "\[24\]" {
            # lasso mode when not clicked on an icon       
            bind .wiring.canvas <Button-1> {if {!$itemFocused} {lasso %x %y}}
            bind .wiring.canvas <B1-Motion> {if {!$itemFocused && !$clicked} {lasso %x %y}}
            bind .wiring.canvas <B1-ButtonRelease> {if {!$itemFocused && !$clicked} {lassoEnd %x %y}}
 
            # pan mode
            bind .wiring.canvas <Shift-Button-1> {.wiring.canvas scan mark %x %y}
            bind .wiring.canvas <Shift-B1-Motion> {.wiring.canvas scan dragto %x %y 1}
        }
    }


    foreach var [variables.#keys] {setM1Binding var $var var$var}
    foreach op [operations.#keys] {setM1Binding op $op op$op}
    foreach id [godleyItems.#keys] {setM1Binding godley $id godley$id}
    foreach id [groupItems.#keys] {setM1Binding group $id group$id}
    foreach id [plots.#keys] {setM1Binding plot $id plot$id}
    foreach id [notes.#keys] {setM1Binding note $id note$id}
}

proc recentreCanvas {} {
    .wiring.canvas xview moveto 0.5
    .wiring.canvas yview moveto 0.5
    .equations.canvas xview moveto 0.5
    .equations.canvas yview moveto 0.5
}

proc anyItems {tag} {
    return [expr [llength [.wiring.canvas find withtag $tag]]>0]
}

proc accessed {items item id} {
    catch {if {$item.id==$id} {return 1}}
    return [$items.hasBeenAccessed $id]
}

proc delIfAccessed {items item id} {
    if [accessed $items $item $id] {.wiring.canvas delete $item$id}
}

proc updateCanvas {} {
    doPushHistory 0
    global fname showPorts
#    .wiring.canvas delete all
    .wiring.canvas delete errorItems
    foreach var [info globals sliderCheck*] {global $var; unset $var}
    setInteractionMode

    # groups need to be done first, as they adjust port positions (hence wires)
    foreach g [groupItems.visibleGroups] {
        delIfAccessed groupItems group $g
        if {[llength [.wiring.canvas find withtag group$g]]==0} {
            group.get $g
            if {[group.group]==-1} {newGroupItem $g}
        }
    }
    groupItems.clearAccessLog

    foreach var [variables.visibleVariables] {
        delIfAccessed variables var $var
        if {[llength [.wiring.canvas find withtag var$var]]==0} {
            var.get $var
            if {[var.group]==-1} {newVar $var}
            drawSlider $var [var.x] [var.y]
        }
    }
    variables.clearAccessLog

    # add operations
    foreach o [operations.visibleOperations] {
        delIfAccessed operations op $o
        if {[llength [.wiring.canvas find withtag op$o]]==0} {
            op.get $o
            if {[op.group]==-1} {drawOperation $o}
        }
    }
    operations.clearAccessLog

    foreach s [switchItems.#keys] {
        delIfAccessed switchItems switch $s
        if {[llength [.wiring.canvas find withtag switchItem$s]]==0} {
            switchItem.get $s
            if {[switchItem.group]==-1} {newSwitch $s}
        } else {
            #redraw
            .wiring.canvas coords switchItem$s [.wiring.canvas coords switchItem$s]
        }
    }
    switchItems.clearAccessLog

    foreach im [plots.#keys] {
        delIfAccessed plots plot $im
        if {[llength [.wiring.canvas find withtag plot$im]]==0} {
            plot.get $im
            newPlotItem $im [plot.x] [plot.y]
        } else {
            #redraw
            .wiring.canvas coords plot$im [.wiring.canvas coords plot$im]
        }
    }
    plots.clearAccessLog

    foreach g [godleyItems.#keys] {
        delIfAccessed godleyItems godley $g
        if {[llength [.wiring.canvas find withtag godley$g]]==0} {
            newGodleyItem $g
        }
    }
    godleyItems.clearAccessLog

    foreach n [notes.#keys] {
        delIfAccessed notes note $n
        if {[llength [.wiring.canvas find withtag note$n]]==0} {
            newNote $n
        }
    }
    notes.clearAccessLog

    # update all wires
    foreach w [visibleWires] {
        wire.get $w
        if {[llength [.wiring.canvas find withtag wire$w]]==0} {
            set id [createWire [wire.coords]]
            newWire $id $w 
        } else {.wiring.canvas coords wire$w [wire.coords]}
    }

# the following loop helps debug port placement
    if {$showPorts} {
        foreach port [ports.#keys] {
            port.get $port
            .wiring.canvas create oval \
                [expr [port.x]-2] [expr [port.y]-2] [expr [port.x]+2] [expr [port.y]+2] \
                -fill {} -outline blue 
        }

    }
  
# debug code to display rectangles indicating bounding boxes for all items
#    update
#    foreach item [.wiring.canvas find all] {
#            .wiring.canvas create rectangle [.wiring.canvas bbox $item] -outline green
#    }

# adjust wire coordinates to where the ports actually are
#    foreach port [ports.#keys] {
#        adjustWire $port
#    }

    # refresh equations
    .equations.canvas itemconfigure eq -tag eq
    doPushHistory 1
}

# mark a canvas item as in error
proc indicateCanvasItemInError {x y} {
    .wiring.canvas create oval [expr $x-15] [expr $y-15] [expr $x+15] [expr $y+15] -outline red -width 2 -tags errorItems
}

menu .wiring.context -tearoff 0

proc doubleClick {item x y} {
    # find out what type of item we're referring to
    set tags [.wiring.canvas gettags $item]
    switch -regexp $tags {
        "variables" {
            set tag [lindex $tags [lsearch -regexp $tags {var[0-9]+}]]
            set id [string range $tag 3 end]
            editItem $id $tag
        }
        "operations" {
            set tag [lindex $tags [lsearch -regexp $tags {op[0-9]+}]]
            set id [string range $tag 2 end]
            editItem $id $tag
        }
    }
}

proc toggleCoupled {id} {
    integral.get $id
    integral.toggleCoupled
    integral.set
    updateCanvas
}

proc addIntegral name {
    set id [addOperation integrate]
    integral.get $id
    integral.description $name
}

# context menu on background canvas
proc canvasContext {x y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help DesignCanvas}
    .wiring.context add command -label "Cut" -command cut
    .wiring.context add command -label "Copy" -command minsky.copy
    .wiring.context add command -label "Save selection as" -command saveSelection
    .wiring.context add command -label "Paste" -command {insertNewGroup [paste]}
    .wiring.context add command -label "Group" -command "minsky.createGroup; updateCanvas"
    tk_popup .wiring.context $x $y
}

proc saveSelection {} {
    global workDir
    set fname [tk_getSaveFile -defaultextension .mky -initialdir $workDir]
    if [string length $fname] {
        saveSelectionAsFile $fname
    }
}

proc canvasHelp {x y} {
    set itemlist [.wiring.canvas find withtag current]
    if {[llength $itemlist]==0} {
        help DesignCanvas
    } else {
        # should only be one or zero current items?
        set canvId [lindex $itemlist 0]
        # assuming that the one true tag is of the form aaaNNN - a is lowercase letter, N a digit
        set tags [.wiring.canvas gettags $canvId]
        set tag [lindex $tags [lsearch -regexp $tags {[a-z]+[0-9]+}]]
        set item [regsub {[0-9]+} $tag ""]
        # extract the item id from its tag
        set id [regsub {[a-z]+} $tag ""]
        $item.get $id

        switch $item {
            var {help Variable}
            op {help "op:[op.name]"}
            godley {help GodleyTable}
            group {help Group}
            plot {help Plot}
        }
    }
}

bind .wiring.canvas <<contextMenu>> {
    set items [.wiring.canvas find withtag current]
    if {[llength $items]==0} {
        canvasContext %X %Y
    } else {
        foreach item $items {
            if {[.wiring.canvas type $item]=="godley"} {
                # TODO - this is so kludgy
                set tags [.wiring.canvas gettags $item]
                set tag [lindex $tags [lsearch -regexp $tags {godley[0-9]+}]]
                set id [string range $tag 6 end]
                rightMouseGodley $id %x %y %X %Y
            } elseif {[.wiring.canvas type $item]=="group"} {
                set tags [.wiring.canvas gettags $item]
                set tag [lindex $tags [lsearch -regexp $tags {group[0-9]+}]]
                set id [string range $tag 5 end]
                rightMouseGroup $id %x %y %X %Y
            } else {
                contextMenu $item %X %Y
            }
        }
    }
}

proc raiseItem {item} {
    .wiring.canvas raise $item all
}
proc lowerItem {item} {
    .wiring.canvas lower $item all
}

# context menu
proc contextMenu {item x y} {
    # find out what type of item we're referring to
    set tags [.wiring.canvas gettags $item]
    switch -regexp $tags {
        "variables" {
            set tag [lindex $tags [lsearch -regexp $tags {var[0-9]+}]]
            set id [string range $tag 3 end]
            var.get $id
	    .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Variable}
            .wiring.context add command -label Description -command "postNote var $id"
            .wiring.context add command -label "Value [var.value]" 
            .wiring.context add command -label "Edit" -command "editItem $id $tag"
            .wiring.context add checkbutton -label "Slider" \
                -command "drawSlider $id $x $y" \
                -variable "sliderCheck$id"
            .wiring.context add command -label "Copy" -command "copyVar $id"
            if {[var.type]=="flow" && ![variables.inputWired [var.valueId]]} {
                .wiring.context add command -label "Add integral" -command "addIntegral [var.name]"
            }
            .wiring.context add command -label "Flip" -command "rotateVar $id 180; flip_default"
            .wiring.context add command -label "Raise" -command "raiseItem var$id"
            .wiring.context add command -label "Lower" -command "lowerItem var$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.variables.@elem $id].*"
	    .wiring.context add command -label "Delete variable" -command "deleteItem $id $tag"
        }
        "operations" {
            set tag [lindex $tags [lsearch -regexp $tags {op[0-9]+}]]
            set id [string range $tag 2 end]
            op.get $id
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command "help op:[op.name]"
            .wiring.context add command -label Description -command "postNote op $id"
            .wiring.context add command -label "Port values [op.portValues]" 
            .wiring.context add command -label "Edit" -command "editItem $id $tag"             
            if {[op.name]=="integrate"} {
                integral.get $id
                .wiring.context add command -label "Copy Var" -command "copyVar [integral.intVarID]"
            }
            if {[op.name]=="constant"} {
                constant.get $id
                global sliderCheck$id
                set sliderCheck$id [constant.sliderVisible]
                .wiring.context add checkbutton -label "Slider" \
                    -command "drawSlider $id $x $y" \
                    -variable "sliderCheck$id"
            }
            if {[op.name]=="data"} {
               .wiring.context add command -label "Import Data" \
                    -command "importData $id" 
            }
            .wiring.context add command -label "Copy" -command "copyOp $id"
            .wiring.context add command -label "Flip" -command "rotateOp $id 180; flip_default"
            op.get $id
            if {[op.name]=="integrate"} {
                .wiring.context add command -label "Toggle var binding" -command "toggleCoupled $id"
            }
            .wiring.context add command -label "Raise" -command "raiseItem op$id"
            .wiring.context add command -label "Lower" -command "lowerItem op$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.operations.@elem $id].*"
            .wiring.context add command -label "Delete operator" -command "deleteItem $id $tag"
        }
        "wires" {
            set tag [lindex $tags [lsearch -regexp $tags {wire[0-9]+}]]
            set id [string range $tag 4 end]
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Wires}
            .wiring.context add command -label Description -command "postNote wire $id"
            .wiring.context add command -label "Straighten" -command "straightenWire $id"
            .wiring.context add command -label "Raise" -command "raiseItem wire$id"
            .wiring.context add command -label "Lower" -command "lowerItem wire$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.wires.@elem $id].*"
            .wiring.context add command -label "Delete wire" -command "deleteItem $id $tag"
        }
        "plots" {
            set tag [lindex $tags [lsearch -regexp $tags {plot[0-9]+}]]
            set id [string range $tag 4 end]
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Plot}
            .wiring.context add command -label Description -command "postNote plot $id"
            .wiring.context add command -label "Expand" -command "plotDoubleClick $id"
            .wiring.context add command -label "Resize" -command "plot::resize $id"
            .wiring.context add command -label "Options" -command "doPlotOptions $id"
            .wiring.context add command -label "Raise" -command "raiseItem plot$id"
            .wiring.context add command -label "Lower" -command "lowerItem plot$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.plots.@elem $id].*"
            .wiring.context add command -label "Delete plot" -command "deletePlot $item $id"
        }
        "godleys" {
            set tag [lindex $tags [lsearch -regexp $tags {godley[0-9]+}]]
            set id [string range $tag 6 end]
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help GodleyTable}
            .wiring.context add command -label Description -command "postNote godley $id"
            .wiring.context add command -label "Open Godley Table" -command "openGodley $id"
            .wiring.context add command -label "Resize Godley" -command "godley::resize $id"
            .wiring.context add command -label "Raise" -command "raiseItem godley$id"
            .wiring.context add command -label "Lower" -command "lowerItem godley$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.godleyItems.@elem $id].*"
            .wiring.context add command -label "Delete Godley Table" -command "deleteItem $id $tag"
        }
        "group" {
            set tag [lindex $tags [lsearch -regexp $tags {group[0-9]+}]]
            set id [string range $tag 5 end]
            groupContext $id $x $y
        }
        "notes" {
            set tag [lindex $tags [lsearch -regexp $tags {note[0-9]+}]]
            set id [string range $tag 4 end]
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Notes}
            .wiring.context add command -label Edit -command "postNote note $id"
            .wiring.context add command -label "Raise" -command "raiseItem note$id"
            .wiring.context add command -label "Lower" -command "lowerItem note$id"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.notes.@elem $id].*"
            .wiring.context add command -label "Delete Note" -command "deleteNote $id; updateCanvas"
        }
        switchItem {
            set tag [lindex $tags [lsearch -regexp $tags {switchItem[0-9]+}]]
            set id [string range $tag 10 end]
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Switches}
            .wiring.context add command -label Description -command "postNote switchItem $id"
            .wiring.context add command -label "Add case" -command "incrCase $id 1" 
            .wiring.context add command -label "Delete case" -command "incrCase $id -1" 
            .wiring.context add command -label "Flip" -command "switchItem.get $id
                       switchItem.flipped [expr ![switchItem.flipped]]
                       redraw switchItem$id"
            .wiring.context add command -label "Raise" -command "raiseItem $tag"
            .wiring.context add command -label "Lower" -command "lowerItem $tag"
            .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.switchItems.@elem $id].*"
            .wiring.context add command -label "Delete Switch" -command "deleteSwitch $id; updateCanvas"
        }
    }
#    .wiring.context post $x $y
    tk_popup .wiring.context $x $y
}

namespace eval godley {
    proc resize {id} {
        godley.get $id
        set bbox [.wiring.canvas bbox godley$id]
        variable orig_width [expr [lindex $bbox 2]-[lindex $bbox 0]]
        variable orig_height [expr [lindex $bbox 3]-[lindex $bbox 1]]
        variable orig_x [godley.x]
        variable orig_y [godley.y]
        set item [eval .wiring.canvas create rectangle $bbox]
        # disable lasso mode
        bind .wiring.canvas <Button-1> ""
        bind .wiring.canvas <B1-Motion> ""
        bind .wiring.canvas <B1-ButtonRelease> ""
        bind .wiring.canvas <Motion> "godley::resizeRect $item %x %y"
        bind .wiring.canvas <ButtonRelease> "godley::resizeItem $item $id %x %y"
    }

        # resize the bounding box to indicate how big we want the icon to be
    proc resizeRect {item x y} {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        variable orig_x
        variable orig_y
        variable orig_width
        variable orig_height
        set w [expr abs($x-$orig_x)]
        set h [expr abs($y-$orig_y)]
        # preserve original aspect ratio
        if {$h/$orig_height>$w/$orig_width} {
            set w [expr $h*$orig_width/$orig_height]
        } else {
            set h [expr $w*$orig_height/$orig_width]
        }            
        .wiring.canvas coords $item  [expr $orig_x-$w] [expr $orig_y-$h] \
            [expr $orig_x+$w] [expr $orig_y+$h]
    }

        # compute width and height and redraw item
    proc resizeItem {item id x y} {
        godley.get $id
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        .wiring.canvas delete $item
        variable orig_width
        variable orig_height
        variable orig_x
        variable orig_y
        set w [expr 2*abs($x-$orig_x)]
        set h [expr 2*abs($y-$orig_y)]
        # preserve original aspect ratio
        if {$h/$orig_height>$w/$orig_width} {
            set z [expr $h/$orig_height]
        } else {
            set z [expr $w/$orig_width]
        }            
        godley.zoom [godley.x] [godley.y] $z

        .wiring.canvas delete godley$id
        newGodleyItem $id
        foreach p [godley.ports]  {
            adjustWire $p
        }
        bind .wiring.canvas <Motion> {}
        bind .wiring.canvas <ButtonRelease> {}
        setInteractionMode
   }
}

proc flip_default {} {
   global globals
   set globals(default_rotation) [expr ($globals(default_rotation)+180)%360]
}

proc deleteItem {id tag} {
    .wiring.canvas delete $tag
    switch -regexp $tag {
        "^op" {
            deleteOperation $id
        }
        "^wire" {
            .wiring.canvas delete handles
            deleteWire $id
            return
        }
        "^var" {
            deleteVariable $id
        }
        "^godley" {
            deleteGodleyTable $id
            destroy .godley$id
        }
        "^note" {
            deleteNote $id
        }
        
    }
    .wiring.canvas delete wires
    updateCanvas
}

proc copyVar {id} {
    global globals
    set newId [copyVariable $id]
    newVar $newId
    var.get $newId
    var.rotation $globals(default_rotation)
    var.set
    placeNewVar $newId
}

proc copyOp  {id} {
    global globals
    set newId [copyOperation $id]
    op.get $id
    op.rotation $globals(default_rotation)
    op.set
    placeNewOp $newId 
}

proc rotateOp {id angle} {
    op.get $id
    op.rotation [expr [op.rotation]+$angle]
    op.set
    drawOperation $id
    foreach p [op.ports] {
        adjustWire $p
    }
}

proc rotateVar {id angle} {
    var.get $id
    var.rotation [expr [var.rotation]+$angle]
    var.set
    newVar $id
    adjustWire [var.outPort]
    adjustWire [var.inPort]
}

proc deiconifyEditVar {} {
    if {![winfo exists .wiring.editVar]} {
        toplevel .wiring.editVar 
        wm resizable .wiring.editVar 0 0
        wm title .wiring.editVar "Edit Variable"
        wm transient .wiring.editVar .wiring

        set row 0
        grid [label .wiring.editVar.title -textvariable editVarInput(title)] -row $row -column 0 -columnspan 999 -pady 10

        global rowdict
        global editVarInput
        set rowdict(Name) 10
        grid [label .wiring.editVar.label10 -text "Name"] -row 10 -column 10 -sticky e
        grid [ttk::combobox  .wiring.editVar.entry10 -textvariable editVarInput(Name)] -row 10 -column 20 -sticky ew -columnspan 2

        set rowdict(Type) 20
        grid [label .wiring.editVar.label20 -text "Type"] -row 20 -column 10 -sticky e
        grid [ttk::combobox  .wiring.editVar.entry20 -textvariable editVarInput(Type) \
                  -state readonly -values "constant parameter flow integral"] \
            -row 20 -column 20 -sticky ew -columnspan 2

        # disable or enable the name field depending on type being selected
        bind .wiring.editVar.entry20 <<ComboboxSelected>> {
            if {[.wiring.editVar.entry20 get]=="constant"} {
                .wiring.editVar.entry10 configure -state disabled
            } else {
                .wiring.editVar.entry10 configure -state enabled
            }
        }
        

        set row 30
        foreach var {
            "Initial Value"
            "Rotation"
            "Short description"
            "Detailed description"
            "Slider Bounds: Max"
            "Slider Bounds: Min"
            "Slider Step Size"
        } {
            set rowdict($var) $row
            grid [label .wiring.editVar.label$row -text $var] -row $row -column 10 -sticky e
            grid [entry  .wiring.editVar.entry$row -textvariable editVarInput($var)] -row $row -column 20 -sticky ew -columnspan 2
            incr row 10
        }
        set editVarInput(initial_focus_value) ".wiring.editVar.entry$rowdict(Initial Value)"
        set editVarInput(initial_focus_rotation) .wiring.editVar.entry$rowdict(Rotation)
        
        frame .wiring.editVar.buttonBar
        button .wiring.editVar.buttonBar.ok -text OK -command {
            scopedDisableEventProcessing
            .wiring.canvas delete all
            convertVarType [var.valueId] $editVarInput(Type)
    
            var.get $editVarInput(id)
            setItem var name {set "editVarInput(Name)"}
            variables.makeVarConsistentWithValue $editVarInput(id)
            setItem var init {set "editVarInput(Initial Value)"}
            setItem var rotation  {set editVarInput(Rotation)}
            setItem var tooltip  {set "editVarInput(Short description)"}
            setItem var detailedText  {set "editVarInput(Detailed description)"}
            setItem var sliderMax  {set "editVarInput(Slider Bounds: Max)"}
            setItem var sliderMin  {set "editVarInput(Slider Bounds: Min)"}
            setItem var sliderStep  {set "editVarInput(Slider Step Size)"}
            setItem var sliderStepRel  {set editVarInput(relative)}
            setSliderProperties $editVarInput(id)
            closeEditWindow .wiring.editVar
            updateCanvas
        }
        # adjust "Slider Step Size" row to include "relative" radiobutton
        set row "$rowdict(Slider Step Size)"
        grid configure .wiring.editVar.entry$row -columnspan 1
        grid [checkbutton .wiring.editVar.checkbox$row -text "relative" -variable "editVarInput(relative)"] -row $row -column 21 -sticky ew -columnspan 1

        button .wiring.editVar.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.editVar}
        pack .wiring.editVar.buttonBar.ok [label .wiring.editVar.buttonBar.spacer -width 2] .wiring.editVar.buttonBar.cancel -side left -pady 10
        grid .wiring.editVar.buttonBar -row 999 -column 0 -columnspan 1000
        bind .wiring.editVar <Key-Return> {invokeOKorCancel .wiring.editVar.buttonBar}
    } else {
        wm deiconify .wiring.editVar
    }
}

proc deiconifyInitVar {} {
    if {![winfo exists .wiring.initVar]} {
        toplevel .wiring.initVar
        wm resizable .wiring.initVar 0 0
        wm title .wiring.initVar "Specify variable name"
        wm transient .wiring.initVar .wiring

        set row 0
        grid [label .wiring.initVar.title -textvariable varInput(title)] -row $row -column 0 -columnspan 999 -pady 10
        frame .wiring.initVar.buttonBar
        button .wiring.initVar.buttonBar.ok -text OK -command "addVariablePostModal"
        button .wiring.initVar.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.initVar}
        pack .wiring.initVar.buttonBar.ok [label .wiring.initVar.buttonBar.spacer -width 2] .wiring.initVar.buttonBar.cancel -side left -pady 10
        grid .wiring.initVar.buttonBar -row 999 -column 0 -columnspan 1000
        bind .wiring.initVar <Key-Return> {invokeOKorCancel .wiring.initVar.buttonBar}
        
        global rowdict
        global varInput initVar_name
        set rowdict(Name) 10
        set initVar_name entry10
        grid [label .wiring.initVar.label10 -text Name] -row 10 -column 10 -sticky e
        grid [ttk::combobox  .wiring.initVar.entry10 -textvariable varInput(Name)] -row 10 -column 20 -sticky ew -columnspan 2

        set rowdict(Type) 20
        grid [label .wiring.initVar.label20 -text "Type"] -row 20 -column 10 -sticky e
        grid [ttk::combobox  .wiring.initVar.entry20 -textvariable varInput(Type) \
                  -state readonly -values "constant parameter flow integral"] \
            -row 20 -column 20 -sticky ew -columnspan 2
        
        # disable or enable the name field depending on type being selected
        bind .wiring.initVar.entry20 <<ComboboxSelected>> {
            if {[.wiring.initVar.entry20 get]=="constant"} {
                .wiring.initVar.entry10 configure -state disabled
            } else {
                .wiring.initVar.entry10 configure -state enabled
            }
        }
        
        set row 30
        foreach var {
            "Value"
            "Rotation"
            "Short description"
            "Detailed description"
            "Slider Bounds: Max"
            "Slider Bounds: Min"
            "Slider Step Size"
        } {
            set rowdict($var) $row
            grid [label .wiring.initVar.label$row -text $var] -row $row -column 10 -sticky e
            grid [entry  .wiring.initVar.entry$row -textvariable varInput($var)] -row $row -column 20 -sticky ew -columnspan 2
            incr row 10
        }
        set varInput(initial_focus) .wiring.initVar.entry$rowdict(Name)
    } else {
        wm deiconify .wiring.initVar
    }
}

proc deiconifyEditConstant {} {
    if {![winfo exists .wiring.editConstant]} {
        toplevel .wiring.editConstant
        wm resizable .wiring.editConstant 0 0
        wm transient .wiring.editConstant .wiring

        global constInput rowdict
        set row 0
        grid [label .wiring.editConstant.title -textvariable constInput(title)] -row $row -column 0 -columnspan 999 -pady 10
        frame .wiring.editConstant.buttonBar
        button .wiring.editConstant.buttonBar.ok -text OK -command {eval $constInput(command)}
        button .wiring.editConstant.buttonBar.cancel -text Cancel -command {eval $constInput(cancelCommand)}
        pack .wiring.editConstant.buttonBar.ok [label .wiring.editConstant.buttonBar.spacer -width 2] .wiring.editConstant.buttonBar.cancel -side left -pady 10
        grid .wiring.editConstant.buttonBar -row 999 -column 0 -columnspan 1000

        foreach var {
            "Name"
            "Value"
            "Rotation"
            "Slider Bounds: Max"
            "Slider Bounds: Min"
            "Slider Step Size"
        } {
            set rowdict($var) $row
            label .wiring.editConstant.label$row -text $var
            entry  .wiring.editConstant.entry$row -textvariable constInput($var)
            incr row 10
        }
        set constInput(initial_focus) .wiring.editConstant.entry$rowdict(Name)
        # setup textvariable for label of "Value"
        set row "$rowdict(Value)"
        .wiring.editConstant.label$row configure -textvariable constInput(ValueLabel)

        # adjust "Slider Step Size" row to include "relative" radiobutton
        set row "$rowdict(Slider Step Size)"
        grid configure .wiring.editConstant.entry$row -columnspan 1
        grid [checkbutton .wiring.editConstant.checkbox$row -text "relative" -variable "constInput(relative)"] -row $row -column 21 -sticky ew -columnspan 1

        bind .wiring.editConstant <Key-Return> {invokeOKorCancel .wiring.editConstant.buttonBar}

    } else {
        wm deiconify .wiring.editConstant
    }
}

proc cleanEditConstantConfig {} {
    global rowdict
    foreach name [array names rowdict] {
        set row $rowdict($name)
        catch {grid remove .wiring.editConstant.label$row .wiring.editConstant.entry$row}
    }
}

proc configEditConstantForConstant {} {
    global rowdict
    cleanEditConstantConfig
    set i 10
    foreach var {
        "Name"
        "Value"
        "Rotation"
        "Slider Bounds: Max"
        "Slider Bounds: Min"
        "Slider Step Size"
    } {
        set row $rowdict($var)
        grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
        grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
        incr i 10
    }
}

proc configEditConstantForIntegral {} {
    global rowdict
    cleanEditConstantConfig
    set i 10
    foreach var {
        "Name"
        "Value"
        "Rotation"
    } {
        set row $rowdict($var)
        grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
        grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
        incr i 10
    }
}

proc configEditConstantForData {} {
    global rowdict
    cleanEditConstantConfig
    set i 10
    foreach var {
        "Name"
        "Rotation"
    } {
        set row $rowdict($var)
        grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
        grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
        incr i 10
    }
}


proc deiconifyEditOperation {} {
    if {![winfo exists .wiring.editOperation]} {
        global opInput
        toplevel .wiring.editOperation
        wm resizable .wiring.editOperation 0 0
        wm title .wiring.editOperation "Edit Operation"
        wm transient .wiring.editOperation .wiring

        frame .wiring.editOperation.buttonBar
        label .wiring.editOperation.title -textvariable opInput(title)
        pack .wiring.editOperation.title -pady 10
        button .wiring.editOperation.buttonBar.ok -text OK -command {
            setItem op rotation {set opInput(Rotation)}
            closeEditWindow .wiring.editOperation
        }
        button .wiring.editOperation.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.editOperation}
        pack .wiring.editOperation.buttonBar.ok [label .wiring.editOperation.buttonBar.spacer -width 2] .wiring.editOperation.buttonBar.cancel -side left -pady 10
        pack .wiring.editOperation.buttonBar -side bottom
        
        bind .wiring.editOperation <Key-Return> {invokeOKorCancel .wiring.editOperation.buttonBar}
        
        frame .wiring.editOperation.rotation
        label .wiring.editOperation.rotation.label -text "Rotation"
        entry  .wiring.editOperation.rotation.value -width 20 -textvariable opInput(Rotation)
        pack .wiring.editOperation.rotation.label .wiring.editOperation.rotation.value -side left
        pack .wiring.editOperation.rotation
        set opInput(initial_focus) .wiring.editOperation.rotation.value
    } else {
        wm deiconify .wiring.editOperation
    }
}
        
# set attribute, and commit to original item
proc setItem {modelCmd attr dialogCmd} {
    global constInput varInput editVarInput opInput
    $modelCmd.$attr [string trim [eval $dialogCmd]]
    $modelCmd.set
}

#proc setInitVal {var dialogCmd} {
#    value.get $var
#    value.init [eval $dialogCmd]
#    value.set $var
#}

proc closeEditWindow {window} {
    grab release $window
    wm withdraw $window
    updateCanvas
}

proc setConstantValue {} {
    global constInput
    constant.value "$constInput(Value)"
    constant.description "$constInput(Name)"
    constant.sliderStepRel "$constInput(relative)"
    constant.sliderMin "$constInput(Slider Bounds: Min)"
    constant.sliderMax "$constInput(Slider Bounds: Max)"
    constant.sliderStep "$constInput(Slider Step Size)"
    constant.sliderBoundsSet 1
}

proc setDataValue {} {
    global constInput
    data.description "$constInput(Name)"
}

proc setIntegralIValue {} {
    global constInput
    integral.description "$constInput(Name)"
    var.get [integral.intVarID]
    value.get [var.valueId]
    setItem value init {set constInput(Value)}
}

proc editItem {id tag} {
    global constInput varInput editVarInput opInput
    switch -regexp $tag {
        "^var" {
            var.get $id
            value.get [var.valueId]
            deiconifyEditVar
            wm title .wiring.editVar "Edit [var.name]"
            # populate combobox with existing variable names
            .wiring.editVar.entry10 configure -values [variables.valueNames]

            set "editVarInput(Name)" [var.name]
            set editVarInput(id) $id
            set "editVarInput(Type)" [var.type]

            set "editVarInput(Initial Value)" [value.init]
            set "editVarInput(Rotation)" [var.rotation]
            set "editVarInput(Slider Bounds: Max)" [var.sliderMax]
            set "editVarInput(Slider Bounds: Min)" [var.sliderMin]
            set "editVarInput(Slider Step Size)" [var.sliderStep]
            set "editVarInput(relative)" [var.sliderStepRel]
            if {[value.godleyOverridden] || [variables.inputWired [var.valueId]]} {
                $editVarInput(initial_focus_value) configure -state disabled  -foreground gray
		::tk::TabToWindow $editVarInput(initial_focus_rotation)
            } else {
                $editVarInput(initial_focus_value) configure -state normal  -foreground black
		::tk::TabToWindow $editVarInput(initial_focus_value)
             }
            set editVarInput(title) "[var.name]: Value=[value.value]"
	    tkwait visibility .wiring.editVar
	    grab set .wiring.editVar
	    wm transient .wiring.editVar
        }
        "^op" {
            op.get $id
            if {[op.name]=="constant" || [op.name]=="integrate" || [op.name]=="data"} {
                set constInput(Value) ""
                set "constInput(Slider Bounds: Min)" ""
                set "constInput(Slider Bounds: Max)" ""
                set "constInput(Slider Step Size)" ""
                deiconifyEditConstant
                switch [op.name] {
                    constant {
                        wm title .wiring.editConstant "Edit Constant"
                        constant.get $id
                        set constInput(Name) [constant.description]
                        set constInput(ValueLabel) "Value"
                        set constInput(Value) [constant.value]
                        constant.initOpSliderBounds
			set "constInput(Slider Bounds: Min)" [constant.sliderMin]
			set "constInput(Slider Bounds: Max)" [constant.sliderMax]
			set "constInput(Slider Step Size)" [constant.sliderStep]
                        set constInput(relative) [constant.sliderStepRel]
                        set setValue setConstantValue
                        configEditConstantForConstant
                    }
                    integrate {
                        wm title .wiring.editConstant "Edit Integral"
                        integral.get $id
                        set constInput(ValueLabel) "Initial Value"
                        var.get [integral.intVarID]
                        value.get [var.valueId]
                        set constInput(Value) [value.init]
                        set setValue setIntegralIValue
                        set constInput(Name) [integral.description]
                        configEditConstantForIntegral
                    }
                    data {
                        wm title .wiring.editConstant "Edit Data Item"
                        data.get $id
                        set setValue setDataValue
                        set constInput(Name) [data.description]
                        configEditConstantForData
                    }
                    
                }
                set constInput(title) $constInput(Name)
                set constInput(Rotation) [op.rotation]
                # value needs to be regotten, as var name may have changed
                set constInput(command) "
                        $setValue
                        setSliderProperties $id
                        setItem op rotation {set constInput(Rotation)}
                        closeEditWindow .wiring.editConstant
                    "
		set constInput(cancelCommand) "closeEditWindow .wiring.editConstant"

		::tk::TabToWindow $constInput(initial_focus);
		tkwait visibility .wiring.editConstant
		grab set .wiring.editConstant
		wm transient .wiring.editConstant

            } else {
                set opInput(title) [op.name]
                set opInput(Rotation) [op.rotation]
                deiconifyEditOperation
		::tk::TabToWindow $opInput(initial_focus);
		tkwait visibility .wiring.editOperation
		grab set .wiring.editOperation
		wm transient .wiring.editOperation
            }
        }
        "^group" {groupEdit $id}
    }
}

proc setVarVal {v x} {
    var.get $v
    var.sliderSet $x
    resetNotNeeded
}

proc setSliderProperties {id} {
    if [winfo  exists .wiring.slider$id] {
        var.get $id
        var.initSliderBounds
        if [var.sliderStepRel] {
            set res [expr [var.sliderStep]*([var.sliderMax]-[var.sliderMin])]
        } else {
            set res [var.sliderStep]
        }
        
        # ensure resolution is accurate enough to not mutate variable value
        set decPos [string first . [var.value]]
        if {$decPos==-1} {
            set newRes 1
        } else {
            set newRes [expr pow(10,1+$decPos-[string len [var.value]])]
        }
       if {$newRes<$res} {set res $newRes}

        # ensure slider does not override value
        var.adjustSliderBounds

        .wiring.slider$id configure -to [var.sliderMax] \
            -from [var.sliderMin] -resolution $res
        if [catch .wiring.slider$id set [var.init]] {
            .wiring.slider$id set [var.value]
        }
    }
}

# if y is the y-coordinate of the constant, then return a y-coordinate
# for an attached slider
proc sliderYCoord {y} {
    return [expr $y-15-10*[zoomFactor]]
}

proc drawSlider {var x y} {
    global sliderCheck$var
    var.get $var
    if {![info exists sliderCheck$var]} {
        # sliderCheck$vae gets initialised to constant.sliderVisible,
        # otherwise sliderCheck$var is more up to date
        set sliderCheck$var [var.sliderVisible]
    }
    if {[var.sliderVisible]!=[set sliderCheck$var]} {
        var.sliderVisible [set sliderCheck$var]
    }

    if {[set sliderCheck$var]} {
        if {![winfo exists .wiring.slider$var]} {
            scale .wiring.slider$var -orient horizontal -width 7 -length 50 \
                -showvalue 1 -sliderlength 30 
        }

        setSliderProperties $var

        # configure command after slider initially set to prevent
        # constant value being set to initial state of slider when
        # constructed.
        .wiring.slider$var configure -command "setVarVal $var"

        #.wiring.canvas bind .wiring.slider$op Keypress-Right 
        bind .wiring.slider$var <Enter> "focus .wiring.slider$var"
        # add shift arrow presses as synonyms for control arrows, as
        # control arrows are interpreted by some window managers
        bind .wiring.slider$var <Shift-KeyPress-Left> \
            "event generate .wiring.slider$var <Control-KeyPress-Left>"
        bind .wiring.slider$var <Shift-KeyPress-Right> \
            "event generate .wiring.slider$var <Control-KeyPress-Right>"
        bind .wiring.slider$var <Shift-KeyPress-Up> \
            "event generate .wiring.slider$var <Control-KeyPress-Up>"
        bind .wiring.slider$var <Shift-KeyPress-Down> \
            "event generate .wiring.slider$var <Control-KeyPress-Down>"
        bind .wiring.slider$var <Leave> {focus .}

        .wiring.canvas create window [var.x] [sliderYCoord [var.y]] -window .wiring.slider$var -tag slider$var
        # this is needed to ensure the setVarVal is fired _before_
        # moving on to processing the next operation in updateCanvas
        update
    } else {
        #remove any slider
        .wiring.canvas delete slider$var
    }
}

proc importData {id} {
    global workDir
    data.get $id
    set f [tk_getOpenFile -multiple 1 -initialdir $workDir]
    if [string length $f] {
        data.readData $f
        updateCanvas
    }
}

proc deiconifyNote {} {
    if {![winfo exists .wiring.note]} {
        toplevel .wiring.note
        frame .wiring.note.tooltip
        label .wiring.note.tooltip.label -text "Short description"
        entry .wiring.note.tooltip.entry -width 40 -justify left
        pack .wiring.note.tooltip.label .wiring.note.tooltip.entry -side left
        text .wiring.note.text -wrap word
        frame .wiring.note.buttons
        button .wiring.note.buttons.cancel -text "Cancel" -command {closeEditWindow .wiring.note}
        button .wiring.note.buttons.ok -text "OK" -command OKnote
        pack .wiring.note.buttons.cancel  .wiring.note.buttons.ok -side left
        pack .wiring.note.tooltip .wiring.note.text .wiring.note.buttons
    } else {
        wm deiconify .wiring.note
    }
}

proc postNote {item id} {
    $item.get $id
    deiconifyNote
    .wiring.note.tooltip.entry delete 0 end
    .wiring.note.tooltip.entry insert 0 [$item.tooltip]
    .wiring.note.text delete 1.0 end
    .wiring.note.text insert 1.0 [$item.detailedText]
    .wiring.note.buttons.ok configure -command "OKnote $item $id"
    tkwait visibility .wiring.note
    grab set .wiring.note
    wm transient .wiring.note
}

proc OKnote {item id} {
    $item.get $id
    $item.tooltip [.wiring.note.tooltip.entry get]
    $item.detailedText  [.wiring.note.text get 1.0 end]
    closeEditWindow .wiring.note
}

proc newNote {id} {
    note.get $id
    .wiring.canvas create text [note.x] [note.y] -text [note.detailedText] -tags "note$id notes"
    setInteractionMode
    .wiring.canvas bind note$id <Double-Button-1> "postNote note $id"
}

proc placeNewNote {} {
    set id [minsky.newNote]
    note.get $id
    note.detailedText "Enter your note here"
    newNote $id
    global moveOffsnote$id.x moveOffsnote$id.y
    set moveOffsnote$id.x 0
    set moveOffsnote$id.y 0
    bind .wiring.canvas <Enter> "move note $id %x %y"
    bind .wiring.canvas <Motion> "move note $id %x %y"
    bind .wiring.canvas <Button-1> clearTempBindings
    bind . <Key-Escape> "clearTempBindings
       deleteNote $id
       .wiring.canvas delete note$id"

}


proc tout {args} {
  puts "$args"
}

# example debugging trace statements
#trace add execution placeNewVar enterstep tout
#trace add execution move enterstep tout
