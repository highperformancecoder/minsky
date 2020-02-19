#  @copyright Steve Keen 2012-2017
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

ttk::frame  .wiring

frame .wiring.menubar 

# create icons for all operations
foreach op [availableOperations] {
    if {$op=="numOps"} break
    # ignore some operations
    switch $op {
        "constant" -
        "copy" -
        "ravel" -
        "integrate"  continue 
    }
    image create photo [set op]Img -file $minskyHome/icons/$op.gif
}


image create photo godleyImg -file $minskyHome/icons/bank.gif
button .wiring.menubar.godley -image godleyImg -height 24 -width 37 \
    -command addGodley
tooltip .wiring.menubar.godley "Godley table"
set helpTopics(.wiring.menubar.godley)  GodleyIcon

image create photo integrateImg -file $minskyHome/icons/integrate.gif
button .wiring.menubar.integrate -image integrateImg -width 37 -height 24 -command {
    addOperation integrate}
tooltip .wiring.menubar.integrate integrate
set helpTopics(.wiring.menubar.integrate) IntOp

image create photo differentiateImg -file $minskyHome/icons/differentiate.gif
button .wiring.menubar.differentiate -image differentiateImg -width 37 -height 24 -command {
    addOperation differentiate}
tooltip .wiring.menubar.differentiate differantiate
set helpTopics(.wiring.menubar.differentiate) Operation:differentiate

button .wiring.menubar.time -image timeImg -width 37 -height 24 -command {
    addOperation time}
tooltip .wiring.menubar.time time
set helpTopics(.wiring.menubar.integrate) Operation:time

button .wiring.menubar.var -text var -foreground #9f0000 -command {
    tk_popup .wiring.menubar.var.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]} 
tooltip .wiring.menubar.var "variable"
set helpTopics(.wiring.menubar.var) Variable

menu .wiring.menubar.var.menu -tearoff 1
.wiring.menubar.var.menu add command -label "variable" -command addVariable
.wiring.menubar.var.menu add command -label "constant" -command addConstant
.wiring.menubar.var.menu add command -label "parameter" -command addParameter

button .wiring.menubar.binops -image addImg -width 37 -height 24 -command {
    tk_popup .wiring.menubar.binops.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]}
tooltip .wiring.menubar.binops "binary operations"
set helpTopics(.wiring.menubar.binops) Operations
menu .wiring.menubar.binops.menu -tearoff 1

button .wiring.menubar.fnops -image sqrtImg -width 37 -height 24 -command {
    tk_popup  .wiring.menubar.fnops.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]}
tooltip .wiring.menubar.fnops "functions"
set helpTopics(.wiring.menubar.fnops) Operations
menu .wiring.menubar.fnops.menu -tearoff 1

button .wiring.menubar.reductionops -image sumImg -width 37 -height 24 -command {
    tk_popup  .wiring.menubar.reductionops.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]}
tooltip .wiring.menubar.reductionops "reductions"
set helpTopics(.wiring.menubar.reductionops) Operations
menu .wiring.menubar.reductionops.menu -tearoff 1

button .wiring.menubar.scanops -image runningSumImg -width 37 -height 24 -command {
    tk_popup  .wiring.menubar.scanops.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]}
tooltip .wiring.menubar.scanops "scans"
set helpTopics(.wiring.menubar.scanops) Operations
menu .wiring.menubar.scanops.menu -tearoff 1

button .wiring.menubar.tensorops -image outerProductImg -width 37 -height 24 -command {
    tk_popup  .wiring.menubar.tensorops.menu [winfo pointerx .wiring.canvas] [winfo pointery .wiring.canvas]}
tooltip .wiring.menubar.tensorops "tensor operations"
set helpTopics(.wiring.menubar.tensorops) Operations
menu .wiring.menubar.tensorops.menu -tearoff 1

pack .wiring.menubar.godley .wiring.menubar.var .wiring.menubar.integrate .wiring.menubar.differentiate -side left -fill y
pack .wiring.menubar.time .wiring.menubar.binops .wiring.menubar.fnops .wiring.menubar.reductionops -side left
pack .wiring.menubar.scanops .wiring.menubar.tensorops -side left

proc addOpMenu {menu op} {
    $menu add command -label $op -image [set op]Img -command "minsky.addOperation $op"
    tooltip $menu -index [$menu index last] $op
}

# create buttons for all available operations (aside from those
# handled especially)
foreach op [availableOperations] {
    if {$op=="numOps"} break
    # ignore some operations
    switch $op {
        "constant" -
        "copy" -
        "ravel" -
        "integrate"  -
        "differentiate" -
        "time" -
        "data" continue 
    }

    switch [classifyOp $op] {
        function {addOpMenu .wiring.menubar.fnops.menu $op}
        binop {addOpMenu .wiring.menubar.binops.menu $op}
        reduction {addOpMenu .wiring.menubar.reductionops.menu $op}
        "scan" {addOpMenu .wiring.menubar.scanops.menu $op}
        tensor {addOpMenu .wiring.menubar.tensorops.menu $op}
        default {
            # shouldn't be here!
        }
    }
}

tooltip .wiring.menubar.var.menu -index 0 "variable toolbar"
tooltip .wiring.menubar.binops.menu -index 0 "binary operations toolbox"
tooltip .wiring.menubar.fnops.menu -index 0 "function toolbar"
tooltip .wiring.menubar.reductionops.menu -index 0 "reduction operations toolbox"
tooltip .wiring.menubar.scanops.menu -index 0 "scans toolbox"
tooltip .wiring.menubar.tensorops.menu -index 0 "tensor toolbox"


button .wiring.menubar.data -image dataImg \
    -height 24 -width 37 -command {addOperation data}
tooltip .wiring.menubar.data "data"
pack .wiring.menubar.data -side left 
set helpTopics(.wiring.menubar.data) "Operation:data"

image create photo switchImg -file $minskyHome/icons/switch.gif


button .wiring.menubar.switch -image switchImg \
    -height 24 -width 37 -command {addSwitch}
tooltip .wiring.menubar.switch "Switch"
pack .wiring.menubar.switch -side left 
set helpTopics(.wiring.menubar.switch) "SwitchIcon"


image create photo plotImg -file $minskyHome/icons/plot.gif
button .wiring.menubar.plot -image plotImg \
    -height 24 -width 37 -command {addPlot}
tooltip .wiring.menubar.plot "PlotWidget"
pack .wiring.menubar.plot -side left 
set helpTopics(.wiring.menubar.plot) "PlotWidget"

image create photo sheetImg -file $minskyHome/icons/sheet.gif
button .wiring.menubar.sheet -image sheetImg \
    -height 24 -width 37 -command {addSheet}
tooltip .wiring.menubar.sheet "Sheet"
pack .wiring.menubar.sheet -side left 
set helpTopics(.wiring.menubar.sheet) "Sheet"

image create photo noteImg -file $minskyHome/icons/note.gif
button .wiring.menubar.note -image noteImg \
    -height 24 -width 37 -command {addNote "Enter your note here"}
tooltip .wiring.menubar.note "Note"
pack .wiring.menubar.note -side left 
set helpTopics(.wiring.menubar.note) "Item"

image create photo ravelImg -file $minskyHome/icons/ravel.gif
button .wiring.menubar.ravel -image ravelImg \
    -height 24 -width 37 -command {addRavel}
tooltip .wiring.menubar.ravel "Ravel"
pack .wiring.menubar.ravel -side left 

pack .wiring.menubar -fill x

# support tooltips
proc hoverMouse {} {
    minsky.canvas.displayDelayedTooltip [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

# reset hoverMouse timer
proc wrapHoverMouse {op x y} {
    after cancel hoverMouse
    # ignore any exceptions
    catch {minsky.canvas.$op $x $y}
    after 3000 hoverMouse
}
    
image create cairoSurface minskyCanvas -surface minsky.canvas
label .wiring.canvas -image minskyCanvas -height $canvasHeight -width $canvasWidth
pack .wiring.canvas -fill both -expand 1
bind .wiring.canvas <ButtonPress-1> {wrapHoverMouse mouseDown %x %y}
bind .wiring.canvas <Control-ButtonPress-1> {wrapHoverMouse controlMouseDown %x %y}
bind .wiring.canvas <ButtonRelease-1> {wrapHoverMouse mouseUp %x %y}
bind .wiring.canvas <Motion> {wrapHoverMouse mouseMove %x %y}
bind .wiring.canvas <Leave> {after cancel hoverMouse}

proc get_pointer_x {c} {
    return [expr {[winfo pointerx $c] - [winfo rootx $c]}]
    #return [winfo pointerx $c]
}

proc get_pointer_y {c} {
    return [expr {[winfo pointery $c] - [winfo rooty $c]}]
    #return [winfo pointery $c]
}

bind . <Key-KP_Add> {zoom 1.1}
bind . <Key-KP_Subtract> {zoom [expr 1.0/1.1]}
# mouse wheel bindings for X11
bind .wiring.canvas <Button-4> {zoomAt %x %y 1.1}
bind .wiring.canvas <Button-5> {zoomAt  %x %y [expr 1.0/1.1]}
# mouse wheel bindings for pc and aqua
bind .wiring.canvas <MouseWheel> { if {%D>=0} {zoomAt %x %y 1.1} {zoomAt  %x %y [expr 1.0/(1.1)]} }

if {[tk windowingsystem]=="aqua"} {
    bind .wiring.canvas <Command-Button-1> {
        tk_messageBox -message "Mouse coordinates %x %y"
    }
} else {
     bind .wiring.canvas <Alt-Button-1> {
        tk_messageBox -message "Mouse coordinates %x %y"
    }
}   

proc zoom {factor} {
    set cbounds [minsky.model.cBounds]
    set x0 [expr 0.5*([lindex $cbounds 0]+[lindex $cbounds 2])]
    set y0 [expr 0.5*([lindex $cbounds 1]+[lindex $cbounds 3])]
    zoomAt $x0 $y0 $factor
}

proc zoomAt {x0 y0 factor} {
    global preferences
    canvas.model.zoom $x0 $y0 $factor
    canvas.requestRedraw
    if $preferences(panopticon) panopticon.requestRedraw
}

.menubar.ops add command -label "Godley Table" -command canvas.addGodley

.menubar.ops add cascade -label "Variable" -menu .wiring.menubar.var.menu
.menubar.ops add cascade -label "Binary Ops" -menu .menubar.ops.binops
.menubar.ops add cascade -label "Functions" -menu .menubar.ops.functions
.menubar.ops add cascade -label "Reductions" -menu .menubar.ops.reductions
.menubar.ops add cascade -label "Scans" -menu .menubar.ops.scans
.menubar.ops add cascade -label "Tensor operations" -menu .menubar.ops.tensors

menu .menubar.ops.binops
menu .menubar.ops.functions
menu .menubar.ops.reductions
menu .menubar.ops.scans
menu .menubar.ops.tensors

foreach op [availableOperations] {
    if {$op=="constant"} continue
    if {$op=="numOps"} break
    set label [regsub {(.*)_$} $op {\1}]
    switch $op {
        "ravel" -
        "integrate"  -
        "differentiate"  -
        "time" -
        "data"  {.menubar.ops add command -label $label -command "minsky.addOperation $op"}
        default {
            switch [classifyOp $op] {
                function {.menubar.ops.functions add command -label $label  -command "minsky.addOperation $op"}
                binop {.menubar.ops.binops add command -label $label  -command "minsky.addOperation $op"}
                reduction {.menubar.ops.reductions add command -label $label  -command "minsky.addOperation $op"}
                "scan" {.menubar.ops.scans add command -label $label  -command "minsky.addOperation $op"}
                tensor {.menubar.ops.tensors add command -label $label  -command "minsky.addOperation $op"}
            }
        }
    }
}

# default command to execute when escape key is pressed
proc handleEscapeKey {} {
    .wiring.context unpost
}
bind . <Key-Escape> {handleEscapeKey}

proc addVariablePostModal {} {
    global globals
    global varInput
    global varType

    set name [string trim $varInput(Name)]
    set varExists [variableValues.count $name]
    minsky.addVariable $name $varInput(Type)
    canvas.itemFocus.init $varInput(Value)
    canvas.itemFocus.setUnits $varInput(Units)
    if {!$varExists} {
        getValue [canvas.itemFocus.valueId]
        canvas.itemFocus.rotation [set varInput(Rotation)]
        canvas.itemFocus.tooltip [set "varInput(Short description)"]
        canvas.itemFocus.detailedText [set "varInput(Detailed description)"]
        canvas.itemFocus.sliderMax  [set "varInput(Slider Bounds: Max)"]
        canvas.itemFocus.sliderMin  [set "varInput(Slider Bounds: Min)"]
        canvas.itemFocus.sliderStep  [set "varInput(Slider Step Size)"]
        canvas.itemFocus.sliderBoundsSet 1
    }
    closeEditWindow .wiring.initVar
}


proc addVariable {} {
    global varInput varType initVar_name
    set varType flow
    set varInput(title) "Create Variable"
    addConstantOrVariable
    .wiring.initVar.$initVar_name configure -state normal
}


proc addConstant {} {
    global varInput varType initVar_name
    set varType constant
    set varInput(title) "Create Constant"
    addConstantOrVariable
    .wiring.initVar.$initVar_name configure -state disabled
}

proc addParameter {} {
    global varInput varType initVar_name
    set varType parameter
    set varInput(title) "Create Parameter"
    addConstantOrVariable
    .wiring.initVar.$initVar_name configure -state normal
}

proc addConstantOrVariable {} {
    global varInput varType
    set varInput(Name) ""
    set varInput(Value) ""
    set varInput(Units) ""
    set varInput(Rotation) [minsky.canvas.defaultRotation]
    set varInput(Type) $varType
    set "varInput(Short description)" ""
    set "varInput(Detailed description)" ""
    deiconifyInitVar
    resetItem
    garbageCollect
    .wiring.initVar.entry10 configure -values [accessibleVars]
    ::tk::TabToWindow $varInput(initial_focus);
    ensureWindowVisible .wiring.initVar
    grab set .wiring.initVar
    wm transient .wiring.initVar
}


# add operation from a keypress
proc addOperationKey {op} {
    addOperation $op
    canvas.mouseUp [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

proc addPlotKey {} {
    addPlot
    canvas.mouseUp [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

proc addNewGodleyItemKey {} {
    addGodley
    canvas.mouseUp [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}
    
# handle arbitrary text typed into the canvas
proc textInput {char} {
    if {$char==""} return

    global textBuffer globals
    #ignore anything unprintable!
    set x [get_pointer_x .wiring.canvas]
    set y [get_pointer_y .wiring.canvas]

    set textBuffer "$char"
    textEntryPopup .textInput $char textOK
    .textInput.entry configure -textvariable textBuffer -takefocus 1
    wm geometry .textInput "+[winfo pointerx .]+[winfo pointery .]"
}

# executed whenever the OK button of textInput is invoked
proc textOK {} {
    global textBuffer
    grab release .textInput
    destroy .textInput
    canvas.moveOffsX 0
    canvas.moveOffsY 0
    if {[lsearch [availableOperations] $textBuffer]>-1} {
        addOperationKey $textBuffer
    } elseif [string match "\[%#\]*" $textBuffer] {
        addNote [string range $textBuffer 1 end]
    } else {
        if [regexp "(.*)=(.*)" $textBuffer dummy name init] {
            minsky.addVariable $name flow
            minsky.canvas.itemFocus.init $init
            minsky.variableValues.reset
        } else {
            minsky.addVariable $textBuffer flow
            
            getItemAtFocus
            editVar
        }
    }
    canvas.mouseUp [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

# operation add shortcuts
bind . <Key-plus> {addOperationKey add}
bind . <Key-minus> {addOperationKey subtract}
bind . <Key-asterisk> {addOperationKey multiply}
bind . <Key-KP_Multiply> {addOperationKey multiply}
bind . <Key-slash> {addOperationKey divide}
bind . <Key-KP_Divide> {addOperationKey divide}
bind . <Key-asciicircum> {addOperationKey pow}
#bind . <Key-backslash> {addOperationKey sqrt}
bind . <Key-ampersand> {addOperationKey integrate}
bind . <Key-equal> {addNewGodleyItemKey}
bind . <Key-at> {addPlotKey}

bind . <Key> {textInput %A}

bind . <Key-Delete> {deleteKey [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}
bind . <Key-BackSpace> {deleteKey  [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}

bind . <KeyPress-Shift_L> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_L> {.wiring.canvas configure -cursor {}}
bind . <KeyPress-Shift_R> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_R> {.wiring.canvas configure -cursor {}}

# slider key bindings
bind . <KeyPress-Left> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 0}
bind . <KeyPress-Right> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 0}
bind . <KeyPress-Up> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 0}
bind . <KeyPress-Down> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 0}
bind . <Shift-KeyPress-Left> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Shift-KeyPress-Right> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Shift-KeyPress-Up> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Shift-KeyPress-Down> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Control-KeyPress-Left> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Control-KeyPress-Right> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Control-KeyPress-Up> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}
bind . <Control-KeyPress-Down> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas] 1}

# handle processing when delete or backspace is pressed
proc deleteKey {x y} {
    if {![canvas.selection.empty]} {
        cut
    } elseif [getItemAt $x $y] {
        canvas.deleteItem
    } elseif [getWireAt $x $y] {
        canvas.deleteWire
    }
}

# global godley icon resource
setGodleyIconResource $minskyHome/icons/bank.svg

proc rightMouseGodley {x y X Y} {
    if [selectVar $x $y] {
        .wiring.context delete 0 end
        .wiring.context add command -label "Copy" -command canvas.copyItem
        .wiring.context add command -label "Rename all instances" -command renameVariableInstances
        .wiring.context post $X $Y
    } else {
        contextMenu $x $y $X $Y
    }
}
# pan mode
bind .wiring.canvas <Shift-Button-1> {
    set panOffsX [expr %x-[minsky.canvas.model.x]]
    set panOffsY [expr %y-[minsky.canvas.model.y]]
}
bind .wiring.canvas <Shift-B1-Motion> {panCanvas [expr %x-$panOffsX] [expr %y-$panOffsY]}

menu .wiring.context -tearoff 0

proc bookmarkAt {x y X Y} {
    # centre x,y in the visible canvas
    set delx [expr 0.5*[.wiring.canvas cget -width]-$x + [minsky.canvas.model.x]]
    set dely [expr 0.5*[.wiring.canvas cget -height]-$y+[minsky.canvas.model.y]]
    minsky.canvas.model.moveTo $delx $dely

    toplevel .getBookmarkName
    wm title .getBookmarkName "Enter bookmark label"
    entry .getBookmarkName.text -width 30
    pack .getBookmarkName.text
    buttonBar .getBookmarkName {
        minsky.canvas.model.addBookmark [.getBookmarkName.text get]
    }
    grab set .getBookmarkName
    wm transient .getBookmarkName
    wm geometry .getBookmarkName +$X+$Y
}

# context menu on background canvas
proc canvasContext {x y X Y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help DesignCanvas}
    .wiring.context add command -label "Cut" -command cut
    .wiring.context add command -label "Copy selection" -command "minsky.copy"
    .wiring.context add command -label "Save selection as" -command saveSelection
    .wiring.context add command -label "Paste selection" -command pasteAt
    if {[getClipboard]==""} {
        .wiring.context entryconfigure end -state disabled
    } 
    .wiring.context add command -label "Bookmark here" -command "bookmarkAt $x $y $X $Y"
    .wiring.context add command -label "Group" -command "minsky.createGroup"
    .wiring.context add command -label "Lock selected Ravels" -command "minsky.canvas.lockRavelsInSelection"
    .wiring.context add command -label "Unlock selected Ravels" -command "minsky.canvas.unlockRavelsInSelection"
    .wiring.context add command -label "Open master group" -command "openModelInCanvas"
    tk_popup .wiring.context $X $Y
}

proc saveSelection {} {
    global workDir
    set f [tk_getSaveFile -defaultextension .mky -initialdir $workDir]
    if [string length $f] {
        set workDir [file dirname $f]
        eval minsky.saveSelectionAsFile {$f}
    }
}


bind .wiring.canvas <Double-Button-1> {doubleButton %x %y}
proc doubleButton {x y} {
    if [getItemAt $x $y] {
        selectVar $x $y
        editItem
    } else {
    # For ticket 1092. Reinstate delete handle user interaction    		
    canvas.delHandle $x $y
    }   
}
# for ticket 1062, new hierarchy of context menu access on mouse right click: wires, items and background canvas.
bind .wiring.canvas <<contextMenu>> {
    if [getWireAt %x %y] {
        wireContextMenu %X %Y  	
    } elseif [getItemAt %x %y] {
        switch [minsky.canvas.item.classType] {
            GodleyIcon {rightMouseGodley %x %y %X %Y}
            Group {rightMouseGroup %x %y %X %Y}
            default {contextMenu %x %y %X %Y}
		}
    } else {
        canvasContext %x %y %X %Y
    }
}

#  proc raiseItem {item} {
#      .wiring.canvas raise $item all
#  }
#  proc lowerItem {item} {
#      .wiring.canvas lower $item all
#  }
#  
proc wireContextMenu {x y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help Wires}
    .wiring.context add command -label Description -command "postNote wire"
    .wiring.context add command -label "Straighten" -command "minsky.canvas.wire.straighten"
#    .wiring.context add command -label "Raise" -command "raiseItem wire$id"
#    .wiring.context add command -label "Lower" -command "lowerItem wire$id"
    .wiring.context add command -label "Browse object" -command "obj_browser minsky.canvas.wire.*"
    .wiring.context add command -label "Delete wire" -command "canvas.deleteWire"
    tk_popup .wiring.context $x $y
}

proc renameVariableInstances {} {
    textEntryPopup .renameDialog [minsky.canvas.item.name] {
        canvas.renameAllInstances [.renameDialog.entry get]
        canvas.requestRedraw
    }
    wm title .renameDialog "Rename [minsky.canvas.item.name]"
}

proc renameIntegralInstances {} {
    textEntryPopup .renameDialog [minsky.canvas.item.description] {
        canvas.renameAllInstances [.renameDialog.entry get]
        canvas.requestRedraw
    }
    wm title .renameDialog "Rename [minsky.canvas.item.description]"
}

proc findDefinition {} {
    set cwidth [.wiring.canvas cget -width]
    set cheight [.wiring.canvas cget -height]
    if [findVariableDefinition] {
        if {abs([minsky.canvas.item.x]-0.5*$cwidth)>0.5*$cwidth ||
            abs([minsky.canvas.item.y]-0.5*$cheight)>0.5*$cheight} {
            # recentre found item
            set offsX [expr [minsky.canvas.model.x]-[minsky.canvas.item.x]+0.5*$cwidth]
            set offsY [expr [minsky.canvas.model.y]-[minsky.canvas.item.y]+0.5*$cheight]
            panCanvas $offsX $offsY
        }
        canvas.itemIndicator 1
    } else {
        tk_messageBox -message "Definition not found"
    }
}

# increment switch cases by delta
proc incrCase {delta} {
    set item minsky.canvas.item
    $item.setNumCases [expr [$item.numCases]+$delta]
    canvas.requestRedraw
}



#  
# context menu
proc contextMenu {x y X Y} {
    set item minsky.canvas.item
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command "help [$item.classType]"
    .wiring.context add command -label Description -command "postNote item"
    # find out what type of item we're referring to
    switch -regex [$item.classType] {
        "Variable*|VarConstant" {
            catch {
                if {[llength [minsky.canvas.item.dims]]==0} {
                    .wiring.context add command -label "Value [minsky.canvas.item.value]"
                } else {
                    .wiring.context add command -label "Dims [minsky.canvas.item.dims]"
                }
            } 
            .wiring.context add command -label "Find definition" -command "findDefinition"
            .wiring.context add command -label "Select all instances" -command {
                canvas.selectAllVariables
            }
            .wiring.context add command -label "Rename all instances" -command {
                renameVariableInstances
            }
            .wiring.context add command -label "Edit" -command "editItem"
            .wiring.context add command -label "Copy item" -command "canvas.copyItem"
            if {![inputWired [$item.valueId]]} {
                .wiring.context add command -label "Add integral" -command "addIntegral"
            }
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
            if {[$item.type]=="parameter"} {
                .wiring.context add command -label "Import CSV" -command CSVImportDialog
            }
            .wiring.context add command -label "Export as CSV" -command exportItemAsCSV
        }
        "Operation*|IntOp|DataOp" {
            set portValues "unknown"
            catch {set portValues [$item.portValues]}
            .wiring.context add command -label "Port values $portValues" 
            .wiring.context add command -label "Edit" -command "editItem"             
            if {[$item.type]=="data"} {
               .wiring.context add command -label "Import Data" \
                    -command "importData" 
               .wiring.context add command -label "Initialise Random" \
                    -command "initRandom" 
            }
            .wiring.context add command -label "Copy item" -command "canvas.copyItem"
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
            if {[$item.type]=="integrate"} {
               .wiring.context add command -label "Toggle var binding" -command "minsky.canvas.item.toggleCoupled; canvas.requestRedraw"
            .wiring.context add command -label "Select all instances" -command {
                canvas.selectAllVariables
            }
            .wiring.context add command -label "Rename all instances" -command {
				renameIntegralInstances
			}	
            }
        }
        "PlotWidget" {
            .wiring.context add command -label "Expand" -command "plotDoubleClick [TCLItem]"
            .wiring.context add command -label "Make Group Plot" -command "$item.makeDisplayPlot"
            .wiring.context add command -label "Options" -command "doPlotOptions $item"
            .wiring.context add command -label "Pen Styles" -command "penStyles $item"
            .wiring.context add command -label "Export as CSV" -command exportItemAsCSV
            .wiring.context add command -label "Export as Image" -command exportItemAsImg
        }
        "GodleyIcon" {
            .wiring.context add command -label "Open Godley Table" -command "openGodley [minsky.openGodley]"
            .wiring.context add command -label "Title" -command {
                textEntryPopup .editGodleyTitle [minsky.canvas.item.table.title] {minsky.canvas.item.table.title [.editGodleyTitle.entry get]; canvas.requestRedraw}
            }
            .wiring.context add command -label "Set currency" -command {
                textEntryPopup .godleyCurrency {} {minsky.canvas.item.setCurrency [.godleyCurrency.entry get]}
            }
            .wiring.context add command -label "Copy flow variables" -command "canvas.copyAllFlowVars"
            .wiring.context add command -label "Copy stock variables" -command "canvas.copyAllStockVars"
            .wiring.context add command -label "Export to file" -command "godley::export"
        }
        "Group" {
            .wiring.context add command -label "Edit" -command "groupEdit"
            .wiring.context add command -label "Open in canvas" -command "openGroupInCanvas"
            .wiring.context add command -label "Zoom to display" -command "canvas.zoomToDisplay"
            .wiring.context add command -label "Remove plot icon" -command "$item.removeDisplayPlot"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            .wiring.context add command -label "Save group as" -command "group::save"
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
            .wiring.context add command -label "Flip Contents" -command "$item.flipContents; canvas.requestRedraw"
            .wiring.context add command -label "Ungroup" -command "canvas.ungroupItem; canvas.requestRedraw"
        }
        "Item" {
            .wiring.context delete 0 end
            .wiring.context add command -label "Copy item" -command "canvas.copyItem"
        }
        SwitchIcon {
            .wiring.context add command -label "Add case" -command "incrCase 1" 
            .wiring.context add command -label "Delete case" -command "incrCase -1" 
            .wiring.context add command -label "Flip" -command "$item.flipped [expr ![minsky.canvas.item.flipped]]; canvas.requestRedraw"
        }
        Ravel {
            .wiring.context add command -label "Export as CSV" -command exportItemAsCSV
            global sortOrder
            set sortOrder [minsky.canvas.item.sortOrder]
            .wiring.context add cascade -label "Axis properties" -menu .wiring.context.axisMenu
            .wiring.context add command -label "Unlock" -command {
                minsky.canvas.item.leaveLockGroup; canvas.requestRedraw
            }
        }
    }

    # common trailer
#            .wiring.context add command -label "Raise" -command "raiseItem $tag"
#            .wiring.context add command -label "Lower" -command "lowerItem $tag"
    .wiring.context add command -label "Browse object" -command "obj_browser minsky.canvas.item.*"
    .wiring.context add command -label "Delete [minsky.canvas.item.classType]" -command "canvas.deleteItem"
    tk_popup .wiring.context $X $Y
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
menu .wiring.context.axisMenu.sort 
.wiring.context.axisMenu add cascade -label "Sort" -menu .wiring.context.axisMenu.sort 
set sortOrder none
foreach order {none forward reverse numForward numReverse} {
    .wiring.context.axisMenu.sort add radiobutton -label $order -command {
        minsky.canvas.item.setSortOrder $order
        minsky.canvas.item.broadcastStateToLockGroup
        reset
    } -value $order -variable sortOrder
}
.wiring.context.axisMenu add command -label "Pick Slices" -command setupPickMenu


proc setDimension {} {
    if {![winfo exists .wiring.context.axisMenu.dim]} {
        toplevel .wiring.context.axisMenu.dim
        wm title .wiring.context.axisMenu.dim "Dimension axis"
        frame .wiring.context.axisMenu.dim.type
        label .wiring.context.axisMenu.dim.type.label -text "type"
        ttk::combobox .wiring.context.axisMenu.dim.type.value -values {string value time} -state readonly -textvariable axisType
        pack .wiring.context.axisMenu.dim.type.label .wiring.context.axisMenu.dim.type.value -side left
        frame .wiring.context.axisMenu.dim.units
        label .wiring.context.axisMenu.dim.units.label -text "units/format"
        tooltip .wiring.context.axisMenu.dim.units.label \
     "Value type: enter a unit string, eg m/s; time type: enter a strftime format string, eg %Y-%m-%d %H:%M:%S, or %Y-Q%Q"
        entry .wiring.context.axisMenu.dim.units.value
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

proc exportItemAsCSV {} {
    global workDir
    set f [tk_getSaveFile -filetypes {
        {"CSV" .csv TEXT} {"All" {.*} TEXT}
    } -initialdir $workDir ]
    if {$f!=""} {
        set workDir [file dirname $f]
        eval minsky.canvas.item.exportAsCSV {$f}
    }
}

proc exportItemAsImg {} {
    global workDir type
    set f [tk_getSaveFile -filetypes [imageFileTypes] -initialdir $workDir -typevariable type ]
    if {$f==""} return
    set workDir [file dirname $f]
    renderImage $f $type minsky.canvas.item
}

namespace eval godley {
    proc export {} {
        global workDir type
        set item minsky.canvas.item
        
        set fname [tk_getSaveFile -filetypes {{"CSV files" .csv TEXT} {"LaTeX files" .tex TEXT}} \
                       -initialdir $workDir -typevariable type]  
        if {$fname==""} return
        set workDir [file dirname $fname]
        if [string match -nocase *.csv "$fname"] {
            eval $item.table.exportToCSV {$fname}
        } elseif [string match -nocase *.tex "$fname"] {
            eval $item.table.exportToLaTeX {$fname}
        } else {
            switch $type {
                "CSV files" {eval $item.table.exportToCSV {$fname.csv}}
                "LaTeX files" {eval $item.table.exportToLaTeX {$fname.tex}}
            }
        }
    }
}

proc flip_default {} {
    minsky.canvas.defaultRotation [expr ([minsky.canvas.defaultRotation]+180)%360]
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
                  -state readonly -values "constant parameter flow integral stock"] \
            -row 20 -column 20 -sticky ew -columnspan 2

        

        set row 30
        foreach var {
            "Initial Value"
            "Units"
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
        set editVarInput(units_focus_value) ".wiring.editVar.entry$rowdict(Units)"
        set editVarInput(initial_focus_rotation) .wiring.editVar.entry$rowdict(Rotation)

        # disable or enable the name field depending on type being selected
        # constants cannot edit name, and neither constant nor flows allow units to be edited
        bind .wiring.editVar.entry20 <<ComboboxSelected>> {
            switch [.wiring.editVar.entry20 get] {
                "constant" {
                    .wiring.editVar.entry10 configure -state disabled
                     #units
                    .wiring.editVar.entry40 configure -state disabled
                }
                "flow" {
                    .wiring.editVar.entry10 configure -state normal
                    .wiring.editVar.entry40 configure -state disabled
                }
                default {
                    .wiring.editVar.entry10 configure -state normal
                    .wiring.editVar.entry40 configure -state normal
                }
            }
        }

        # initialise variable type when selected from combobox
        bind .wiring.editVar.entry10 <<ComboboxSelected>> {
            getValue [minsky.canvas.item.valueIdInCurrentScope [.wiring.editVar.entry10 get]]
            .wiring.editVar.entry20 set [value.type]
        }
        
        frame .wiring.editVar.buttonBar
        button .wiring.editVar.buttonBar.ok -text OK -command {
            set item minsky.canvas.item
            $item.init $editVarInput(Initial Value)
            $item.setUnits $editVarInput(Units)
            $item.rotation  $editVarInput(Rotation)
            $item.tooltip  $editVarInput(Short description)
            $item.detailedText  $editVarInput(Detailed description)
            $item.sliderMax  $editVarInput(Slider Bounds: Max)
            $item.sliderMin  $editVarInput(Slider Bounds: Min)
            $item.sliderStep  $editVarInput(Slider Step Size)
            $item.sliderStepRel  $editVarInput(relative)
            retypeItem $editVarInput(Type)
            # update name after type change, as it is needed when changing from constants. (ticket #1135)
            $item.name $editVarInput(Name)
                
            makeVariablesConsistent
            catch reset
            closeEditWindow .wiring.editVar
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
        bind .wiring.editVar <Key-Escape> {.wiring.editVar.buttonBar.cancel invoke}
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
        bind .wiring.initVar <Key-Escape> {.wiring.initVar.buttonBar.cancel invoke}

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
                .wiring.initVar.entry10 configure -state normal
            }
        }

        # initialise variable type when selected from combobox
        bind .wiring.initVar.entry10 <<ComboboxSelected>> {
            getValue [.wiring.initVar.entry10 get]
            .wiring.initVar.entry20 set [value.type]
        }
        
        set row 30
        foreach var {
            "Value"
            "Units"
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
            "Units"
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
        bind .wiring.editConstant <Key-Escape> {.wiring.editConstant.buttonBar.cancel invoke}

    } else {
        wm deiconify .wiring.editConstant
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
            minsky.canvas.item.rotation [set opInput(Rotation)]
            minsky.canvas.item.axis [set opInput(Axis)]
            minsky.canvas.item.arg [set opInput(Argument)]
            closeEditWindow .wiring.editOperation
        }
        button .wiring.editOperation.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.editOperation}
        bind .wiring.editOperation <Key-Return> {invokeOKorCancel .wiring.editOperation.buttonBar}
        bind .wiring.editOperation <Key-Escape> {.wiring.editOperation.buttonBar.cancel invoke}
        pack .wiring.editOperation.buttonBar.ok [label .wiring.editOperation.buttonBar.spacer -width 2] .wiring.editOperation.buttonBar.cancel -side left -pady 10
        pack .wiring.editOperation.buttonBar -side bottom
        
        
        frame .wiring.editOperation.rotation
        label .wiring.editOperation.rotation.label -text "Rotation" -width 10
        entry  .wiring.editOperation.rotation.value -width 20 -textvariable opInput(Rotation)
        pack .wiring.editOperation.rotation.value .wiring.editOperation.rotation.label -side right
        pack .wiring.editOperation.rotation
        set opInput(initial_focus) .wiring.editOperation.rotation.value

        frame .wiring.editOperation.axis
        label .wiring.editOperation.axis.label -text "Axis" -width 10
        ttk::combobox  .wiring.editOperation.axis.value  -width 20 -textvariable opInput(Axis)
        pack .wiring.editOperation.axis.value .wiring.editOperation.axis.label -side right
        pack .wiring.editOperation.axis
        tooltip .wiring.editOperation.axis.label "Some tensor operations operate along a particular axis"
        
        frame .wiring.editOperation.arg
        label .wiring.editOperation.arg.label -text "Argument" -width 10
        ttk::combobox  .wiring.editOperation.arg.value  -width 20 -textvariable opInput(Argument)
        pack .wiring.editOperation.arg.value .wiring.editOperation.arg.label -side right
        pack .wiring.editOperation.arg

        tooltip .wiring.editOperation.arg.label "Some operations have an argument, such as the difference operation"
    } else {
        wm deiconify .wiring.editOperation
    }
    .wiring.editOperation.axis.value configure -values [minsky.canvas.item.dimensions]
    set opInput(Axis) [minsky.canvas.item.axis]
    set opInput(Argument) [minsky.canvas.item.arg]
}

proc closeEditWindow {window} {
    grab release $window
    destroy $window
}

proc editVar {} {
    global editVarInput
    set item minsky.canvas.item
    getValue [$item.valueId]
    deiconifyEditVar
    wm title .wiring.editVar "Edit [$item.name]"
    # populate combobox with existing variable names
    .wiring.editVar.entry10 configure -values [accessibleVars]

    set "editVarInput(Name)" [$item.name]
    set "editVarInput(Type)" [$item.type]

    set "editVarInput(Initial Value)" [$item.init]
    set "editVarInput(Units)" [value.units.str]
    set "editVarInput(Rotation)" [$item.rotation]
    set "editVarInput(Slider Bounds: Max)" [$item.sliderMax]
    set "editVarInput(Slider Bounds: Min)" [$item.sliderMin]
    set "editVarInput(Slider Step Size)" [$item.sliderStep]
    set "editVarInput(relative)" [$item.sliderStepRel]
    set "editVarInput(Short description)" [$item.tooltip]
    set "editVarInput(Detailed description)" [$item.detailedText]
    if {[minsky.value.godleyOverridden] || [inputWired [$item.valueId]]} {
        $editVarInput(initial_focus_value) configure -state disabled  -foreground gray
        ::tk::TabToWindow $editVarInput(initial_focus_rotation)
    } else {
        $editVarInput(initial_focus_value) configure -state normal  -foreground black
        ::tk::TabToWindow $editVarInput(initial_focus_value)
    }
    switch [$item.type] {
        "constant" -
        "flow" {$editVarInput(units_focus_value) configure -state disabled  -foreground gray}
    }

    set value "unknown"
    catch {set value [value.value]}
    set editVarInput(title) "[$item.name]: Value=$value"
    ensureWindowVisible .wiring.editVar
    grab set .wiring.editVar
    wm transient .wiring.editVar
}

proc setDataValue {} {
    global constInput
    set item minsky.canvas.item
    $item.description "$constInput(Name)"
    $item.rotation $constInput(Rotation)
}

proc setIntegralIValue {} {
    global constInput
    set item minsky.canvas.item
    $item.description $constInput(Name)
    # description may have change intVar, so use value instead to set init
    getValue [$item.valueId]
    value.init $constInput(Value)
    value.setUnits $constInput(Units)
    $item.rotation $constInput(Rotation)
}

proc cleanEditConstantConfig {} {
    global rowdict
    foreach name [array names rowdict] {
        set row $rowdict($name)
        catch {grid remove .wiring.editConstant.label$row .wiring.editConstant.entry$row}
    }
}

proc configEditConstantForIntegral {} {
    global rowdict
    cleanEditConstantConfig
    set i 10
    foreach var {
        "Name"
        "Value"
        "Units"
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

proc editItem {} {
    global constInput varInput editVarInput opInput
    set item minsky.canvas.item
    switch -regexp [$item.classType] {
        "Variable*|VarConstant" {editVar}
        "Operation*" {
            set opType [minsky.canvas.item.type]
            set opInput(title) [minsky.canvas.item.type]
            set opInput(Rotation) [minsky.canvas.item.rotation]
            deiconifyEditOperation
            ::tk::TabToWindow $opInput(initial_focus);
            ensureWindowVisible .wiring.editOperation
            grab set .wiring.editOperation
            wm transient .wiring.editOperation
        }
        "IntOp|DataOp" {
            set constInput(Value) ""
            set "constInput(Slider Bounds: Min)" ""
            set "constInput(Slider Bounds: Max)" ""
            set "constInput(Slider Step Size)" ""
            deiconifyEditConstant
            wm title .wiring.editConstant "Edit Integral"
            set constInput(ValueLabel) "Initial Value"
            if {[$item.classType]=="IntOp"} {
                set constInput(Value) [$item.intVar.init]
                set constInput(Units) [$item.intVar.unitsStr]
                set setValue setIntegralIValue
                configEditConstantForIntegral
            } else {
                set setValue setDataValue
                configEditConstantForData
            }
            set constInput(Name) [$item.description]
            set constInput(title) $constInput(Name)
            set constInput(Rotation) [$item.rotation]
            # value needs to be regotten, as var name may have changed
            set constInput(command) "
                        $setValue
                        closeEditWindow .wiring.editConstant
                    "
            set constInput(cancelCommand) "closeEditWindow .wiring.editConstant"

            ::tk::TabToWindow $constInput(initial_focus);
            ensureWindowVisible .wiring.editConstant
            grab set .wiring.editConstant
            wm transient .wiring.editConstant
        }
        "Group" {groupEdit}
        "GodleyIcon" {openGodley [minsky.openGodley]}
        # plot widgets are slightly different, in that double-click
        # expands the plot, rather than edits.
        "PlotWidget" {plotDoubleClick [TCLItem]}
        "Item" {postNote item}
    }
}

proc importData {} {
    global workDir
    set f [tk_getOpenFile -multiple 1 -initialdir $workDir]
    if [string length $f] {
        set workDir [file dirname $f]
        eval minsky.canvas.item.readData {$f}
    }
}

proc initRandom {} {
    if {![winfo exists .wiring.initRandom]} {
        toplevel .wiring.initRandom
        frame .wiring.initRandom.xmin
        label .wiring.initRandom.xmin.label -text "min x value"
        entry .wiring.initRandom.xmin.entry -width 40 -justify left
        pack .wiring.initRandom.xmin.label .wiring.initRandom.xmin.entry -side left
        .wiring.initRandom.xmin.entry insert 0 "0"
        frame .wiring.initRandom.xmax
        label .wiring.initRandom.xmax.label -text "max x value"
        entry .wiring.initRandom.xmax.entry -width 40 -justify left
        pack .wiring.initRandom.xmax.label .wiring.initRandom.xmax.entry -side left
        .wiring.initRandom.xmax.entry insert 0 "1"
        frame .wiring.initRandom.numVals
        label .wiring.initRandom.numVals.label -text "number of samples"
        entry .wiring.initRandom.numVals.entry -width 40 -justify left
        .wiring.initRandom.numVals.entry insert 0 "100"
        pack .wiring.initRandom.numVals.label .wiring.initRandom.numVals.entry -side left
        frame .wiring.initRandom.buttonBar
        button .wiring.initRandom.buttonBar.cancel -text "Cancel" -command {closeEditWindow .wiring.initRandom}
        button .wiring.initRandom.buttonBar.ok  -text "OK" -command doInitRandom
        pack .wiring.initRandom.buttonBar.cancel  .wiring.initRandom.buttonBar.ok -side left
        pack .wiring.initRandom.xmin .wiring.initRandom.xmax .wiring.initRandom.numVals .wiring.initRandom.buttonBar
        bind .wiring.initRandom <Key-Escape> {.wiring.initRandom.buttonBar.cancel invoke}
        bind .wiring.initRandom <Key-Return> {.wiring.initRandom.buttonBar.ok invoke}
    } else {
        deiconify .wiring.initRandom
    }
    ensureWindowVisible .wiring.initRandom
    grab set .wiring.initRandom
    wm transient .wiring.initRandom
}

proc doInitRandom {} {
    minsky.canvas.item.initRandom [.wiring.initRandom.xmin.entry get] [.wiring.initRandom.xmax.entry get] [.wiring.initRandom.numVals.entry get]
    closeEditWindow .wiring.initRandom
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
        bind .wiring.note <Key-Escape> {.wiring.note.buttons.cancel invoke}
        pack .wiring.note.buttons.cancel  .wiring.note.buttons.ok -side left
        pack .wiring.note.tooltip .wiring.note.text .wiring.note.buttons
    } else {
        wm deiconify .wiring.note
    }
}

proc postNote {item} {
    deiconifyNote
    .wiring.note.tooltip.entry delete 0 end
    .wiring.note.tooltip.entry insert 0 [minsky.canvas.$item.tooltip]
    .wiring.note.text delete 1.0 end
    .wiring.note.text insert 1.0 [minsky.canvas.$item.detailedText]
    .wiring.note.buttons.ok configure -command "OKnote $item"
    ensureWindowVisible .wiring.note
    grab set .wiring.note
    wm transient .wiring.note
}

proc OKnote {item} {
    minsky.canvas.$item.tooltip [.wiring.note.tooltip.entry get]
    minsky.canvas.$item.detailedText  [string trim [.wiring.note.text get 1.0 end]]
    closeEditWindow .wiring.note
}

proc tout {args} {
  puts "$args"
}

#  # example debugging trace statements
#  #trace add execution placeNewVar enterstep tout
#  #trace add execution move enterstep tout
#  
