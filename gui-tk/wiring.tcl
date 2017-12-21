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

set menubarLine 0
ttk::frame .wiring.menubar.line0

image create photo godleyImg -file $minskyHome/icons/bank.gif
button .wiring.menubar.line0.godley -image godleyImg -height 24 -width 37 \
    -command addGodley
tooltip .wiring.menubar.line0.godley "Godley table"
set helpTopics(.wiring.menubar.line0.godley)  GodleyIcon

button .wiring.menubar.line0.var -text var -command addVariable -foreground #9f0000
tooltip .wiring.menubar.line0.var "variable"
set helpTopics(.wiring.menubar.line0.var) Variable

button .wiring.menubar.line0.const -text const -command addConstant -foreground #00007f
tooltip .wiring.menubar.line0.const "constant"
set helpTopics(.wiring.menubar.line0.const) Variable:constant

button .wiring.menubar.line0.parameter -text param -command addParameter -foreground #00007f
tooltip .wiring.menubar.line0.parameter "parameter"
set helpTopics(.wiring.menubar.line0.const) Variable:parameter

image create photo integrateImg -file $minskyHome/icons/integrate.gif
button .wiring.menubar.line0.integrate -image integrateImg -command {
    addOperation integrate}
tooltip .wiring.menubar.line0.integrate integrate
set helpTopics(.wiring.menubar.line0.integrate) Integrate

pack .wiring.menubar.line0.godley .wiring.menubar.line0.var .wiring.menubar.line0.const .wiring.menubar.line0.parameter .wiring.menubar.line0.integrate -side left

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
    button .wiring.menubar.line$menubarLine.$op -image [set op]Img -command "minsky.addOperation $op" -height 24 -width 24
    tooltip .wiring.menubar.line$menubarLine.$op $opTrimmed

    pack .wiring.menubar.line$menubarLine.$op -side left 
    set helpTopics(.wiring.menubar.line$menubarLine.$op) "Operation:$op"
}

if {[tk windowingsystem]=="aqua"} {
    image create photo switchImg -file $minskyHome/icons/switch.gif
} else {
    image create photo switchImg -width 24 -height 24
    operationIcon switchImg switch
}
button .wiring.menubar.line$menubarLine.switch -image switchImg \
    -height 24 -width 37 -command {addSwitch}
tooltip .wiring.menubar.line$menubarLine.switch "Switch"
pack .wiring.menubar.line$menubarLine.switch -side left 
set helpTopics(.wiring.menubar.line$menubarLine.switch) "SwitchIcon"


image create photo plotImg -file $minskyHome/icons/plot.gif
button .wiring.menubar.line$menubarLine.plot -image plotImg \
    -height 24 -width 37 -command {addPlot}
tooltip .wiring.menubar.line$menubarLine.plot "PlotWidget"
pack .wiring.menubar.line$menubarLine.plot -side left 
set helpTopics(.wiring.menubar.line$menubarLine.plot) "PlotWidget"

image create photo noteImg -file $minskyHome/icons/note.gif
button .wiring.menubar.line$menubarLine.note -image noteImg \
    -height 24 -width 37 -command {addNote "Enter your note here"}
tooltip .wiring.menubar.line$menubarLine.note "Note"
pack .wiring.menubar.line$menubarLine.note -side left 
set helpTopics(.wiring.menubar.line$menubarLine.note) "Item"

# pack menubar lines
for {set i 0} {$i<=$menubarLine} {incr i} {
    pack .wiring.menubar.line$i -side top -anchor w
}
pack .wiring.menubar -fill x

image create cairoSurface minskyCanvas -surface minsky.canvas
label .wiring.canvas -image minskyCanvas -height $canvasHeight -width $canvasWidth
pack .wiring.canvas -fill both -expand 1
bind .wiring.canvas <ButtonPress-1> {minsky.canvas.mouseDown %x %y}
bind .wiring.canvas <ButtonRelease-1> {minsky.canvas.mouseUp %x %y}
bind .wiring.canvas <Motion> {minsky.canvas.mouseMove %x %y}

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
    set x0 [get_pointer_x .wiring.canvas]
    set y0 [get_pointer_y .wiring.canvas]
    zoomAt $x0 $y0 $factor
}

proc zoomAt {x0 y0 factor} {
    if {$factor>1} {
        model.zoom $x0 $y0 $factor
    } else {
        model.zoom $x0 $y0 $factor
    }
    canvas.requestRedraw
    panopticon.requestRedraw
}

.menubar.ops add command -label "Godley Table" -command canvas.addGodley

.menubar.ops add command -label "Variable" -command "addVariable" 
foreach var [availableOperations] {
    if {$var=="constant"} continue
    if {$var=="numOps"} break
    .menubar.ops add command -label [regsub {(.*)_$} $var {\1}] -command "minsky.addOperation $var"
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
    .wiring.initVar.$initVar_name configure -state enabled
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
    .wiring.initVar.$initVar_name configure -state enabled
}

proc addConstantOrVariable {} {
    global varInput varType
    set varInput(Name) ""
    set varInput(Value) ""
    set varInput(Type) $varType
    set "varInput(Short description)" ""
    set "varInput(Detailed description)" ""
    deiconifyInitVar
    resetItem
    .wiring.initVar.entry10 configure -values [accessibleVars]
    ::tk::TabToWindow $varInput(initial_focus);
    tkwait visibility .wiring.initVar
    grab set .wiring.initVar
    wm transient .wiring.initVar
}


# add operation from a keypress
proc addOperationKey {op} {
    addOperation $op
    canvas.mouseUp [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
}

# handle arbitrary text typed into the canvas
proc textInput {char} {
    if {$char==""} return

    global textBuffer globals
    #ignore anything unprintable!
    set x [get_pointer_x .wiring.canvas]
    set y [get_pointer_y .wiring.canvas]

    if {![winfo exists .textInput]} {
        set textBuffer ""
        toplevel .textInput
        entry .textInput.entry -textvariable textBuffer -takefocus 1
        .textInput.entry insert 0 $char
        frame .textInput.buttonBar
        button .textInput.buttonBar.ok -text "OK" -command textOK
        button .textInput.buttonBar.cancel -text "Cancel" -command {
            grab release .textInput
            destroy .textInput
        }
        pack .textInput.buttonBar.cancel .textInput.buttonBar.ok
        pack .textInput.entry .textInput.buttonBar -side top
        bind .textInput <Key-Return> {.textInput.buttonBar.ok invoke}
        bind .textInput <Key-Escape> {.textInput.buttonBar.cancel invoke}
        wm geometry .textInput "+[winfo pointerx .]+[winfo pointery .]"
        focus .textInput.entry
        tkwait visibility .textInput
        # reset cursor in the case a shifted key has been pressed
        .wiring.canvas configure -cursor arrow
        grab set .textInput
        wm transient .textInput
    }
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
            
            getItemAt [minsky.canvas.itemFocus.x] \
                [minsky.canvas.itemFocus.y]
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
bind . <Key-at> {newPlotItem [plots.nextPlotID] [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}

bind . <Key> {textInput %A}

bind . <Key-Delete> {deleteKey [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}
bind . <Key-BackSpace> {deleteKey  [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}

bind . <KeyPress-Shift_L> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_L> {.wiring.canvas configure -cursor arrow}
bind . <KeyPress-Shift_R> {.wiring.canvas configure -cursor $panIcon}
bind . <KeyRelease-Shift_R> {.wiring.canvas configure -cursor arrow}

# slider key bindings
bind . <KeyPress-Left> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}
bind . <KeyPress-Right> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}
bind . <KeyPress-Up> {canvas.handleArrows 1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}
bind . <KeyPress-Down> {canvas.handleArrows -1 [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]}

# handle processing when delete or backspace is pressed
proc deleteKey {x y} {
    if {![canvas.selection.empty]} {
        cut
    } elseif [getItemAt $x $y] {
        canvas.deleteItem
    } elseif [getWireAt $x $y] {
        canvas.deleteWire
    }
    canvas.requestRedraw
}

# global godley icon resource
setGodleyIconResource $minskyHome/icons/bank.svg

proc rightMouseGodley {x y X Y} {
    if [selectVar $x $y] {
        .wiring.context delete 0 end
        .wiring.context add command -label "Edit" -command "editItem"
        .wiring.context add command -label "Copy" -command "canvas.copyItem"
        .wiring.context post $X $Y
    } else {
        contextMenu $x $y $X $Y
    }
}
# pan mode
bind .wiring.canvas <Shift-Button-1> {set panOffsX [expr %x-[model.x]]; set panOffsY [expr %y-[model.y]]}
bind .wiring.canvas <Shift-B1-Motion> {panCanvases [expr %x-$panOffsX] [expr %y-$panOffsY]}

menu .wiring.context -tearoff 0

# context menu on background canvas
proc canvasContext {x y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help DesignCanvas}
    .wiring.context add command -label "Cut" -command cut
    .wiring.context add command -label "Copy" -command minsky.copy
    .wiring.context add command -label "Save selection as" -command saveSelection
    .wiring.context add command -label "Paste" -command {paste}
    .wiring.context add command -label "Group" -command "minsky.createGroup"
    .wiring.context add command -label "Open master group" -command "openModelInCanvas"
    tk_popup .wiring.context $x $y
}


bind .wiring.canvas <Double-Button-1> {doubleButton %x %y}
proc doubleButton {x y} {
    if [getItemAt $x $y] {
        selectVar $x $y
        editItem
    }
}

bind .wiring.canvas <<contextMenu>> {
    if [getItemAt %x %y] {
        switch [minsky.canvas.item.classType] {
            GodleyIcon {rightMouseGodley %x %y %X %Y}
            Group {rightMouseGroup %x %y %X %Y}
            default {contextMenu %x %y %X %Y}
        }
    } elseif [getWireAt %x %y] {
        wireContextMenu %X %Y
    } else {
        canvasContext  %X %Y
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
    .wiring.context add command -label "Browse object" -command "obj_browser canvas.wire.*"
    .wiring.context add command -label "Delete wire" -command "canvas.deleteWire"
    tk_popup .wiring.context $x $y
}

toplevel .renameDialog
label .renameDialog.title
entry .renameDialog.newName
frame .renameDialog.buttonBar
button .renameDialog.buttonBar.cancel -text cancel -command {
    grab release .renameDialog
    wm withdraw .renameDialog
}
button .renameDialog.buttonBar.ok -text OK -command {
    canvas.renameAllInstances [.renameDialog.newName get]
    canvas.requestRedraw
    grab release .renameDialog
    wm withdraw .renameDialog
}
pack .renameDialog.buttonBar.cancel .renameDialog.buttonBar.ok -side left
pack .renameDialog.title .renameDialog.newName .renameDialog.buttonBar
bind .renameDialog <Key-Return> {.renameDialog.buttonBar.ok invoke}
bind .renameDialog <Key-Escape> {.renameDialog.buttonBar.cancel invoke}
wm withdraw .renameDialog


proc renameVariableInstances {} {
    .renameDialog.title configure -text "Rename [minsky.canvas.item.name]"
    .renameDialog.newName delete 0 end
    wm deiconify .renameDialog
    ::tk::TabToWindow .renameDialog.newName
    tkwait visibility .renameDialog
    grab set .renameDialog
    wm transient .renameDialog
}


proc findDefinition {} {
    if [canvas.findVariableDefinition] {
        canvas.indicateItem
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
    # find out what type of item we're referring to
    switch -regex [$item.classType] {
        "Variable*|VarConstant" {
            .wiring.context add command -label Description -command "postNote item"
            catch {.wiring.context add command -label "Value [minsky.canvas.item.value]"} 
            .wiring.context add command -label "Find definition" -command "findDefinition"
            .wiring.context add command -label "Select all instances" -command {
                canvas.selectAllVariables
            }
            .wiring.context add command -label "Rename all instances" -command {
                renameVariableInstances
            }
            .wiring.context add command -label "Edit" -command "editItem"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            if {[$item.type]=="flow" && ![inputWired [$item.valueId]]} {
                .wiring.context add command -label "Add integral" -command "addIntegral"
            }
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
        }
        "Operation*|IntOp|DataOp" {
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Port values [$item.portValues]" 
            .wiring.context add command -label "Edit" -command "editItem"             
            if {[$item.type]=="data"} {
               .wiring.context add command -label "Import Data" \
                    -command "importData" 
               .wiring.context add command -label "Initialise Random" \
                    -command "initRandom" 
            }
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
            if {[$item.type]=="integrate"} {
                .wiring.context add command -label "Toggle var binding" -command "minsky.canvas.item.toggleCoupled; canvas.requestRedraw"
            .wiring.context add command -label "Select all instances" -command {
                canvas.selectAllVariables
            }
            .wiring.context add command -label "Rename all instances" -command {
                renameVariableInstances
            }
            }
        }
        "PlotWidget" {
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Expand" -command "plotDoubleClick [TCLItem]"
            .wiring.context add command -label "Make Group Plot" -command "$item.makeDisplayPlot"
            .wiring.context add command -label "Resize" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Options" -command "doPlotOptions $item"
        }
        "GodleyIcon" {
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Open Godley Table" -command "openGodley"
            .wiring.context add command -label "Copy flow variables" -command "canvas.copyAllFlowVars"
            .wiring.context add command -label "Copy stock variables" -command "canvas.copyAllStockVars"
            .wiring.context add command -label "Resize Godley" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Export to file" -command "godley::export"
        }
        "Group" {
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Edit" -command "groupEdit"
            .wiring.context add command -label "Open in canvas" -command "openGroupInCanvas"
            .wiring.context add command -label "Zoom to display" -command "canvas.zoomToDisplay"
            .wiring.context add command -label "Remove plot icon" -command "$item.removeDisplayPlot"
            .wiring.context add command -label "Resize" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            .wiring.context add command -label "Save group as" -command "group::save"
            .wiring.context add command -label "Flip" -command "$item.flip; flip_default"
            .wiring.context add command -label "Flip Contents" -command "$item.flipContents; canvas.requestRedraw"
            .wiring.context add command -label "Ungroup" -command "canvas.ungroupItem; canvas.requestRedraw"
        }
        "Item" {
            .wiring.context delete 0 end
            .wiring.context add command -label Edit -command "postNote item"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
        }
        SwitchIcon {
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Add case" -command "incrCase 1" 
            .wiring.context add command -label "Delete case" -command "incrCase -1" 
            .wiring.context add command -label "Flip" -command "$item.flipped [expr ![minsky.canvas.item.flipped]]; canvas.requestRedraw"
        }
    }

    # common trailer
#            .wiring.context add command -label "Raise" -command "raiseItem $tag"
#            .wiring.context add command -label "Lower" -command "lowerItem $tag"
    .wiring.context add command -label "Browse object" -command "obj_browser minsky.canvas.item.*"
    .wiring.context add command -label "Delete [minsky.canvas.item.classType]" -command "canvas.deleteItem"
    tk_popup .wiring.context $X $Y
}
#  
namespace eval godley {
    proc export {} {
        global workDir type
        set item minsky.canvas.item
        
        set fname [tk_getSaveFile -filetypes {{"CSV files" csv TEXT} {"LaTeX files" tex TEXT}} \
                       -initialdir $workDir -typevariable type]  
        if {$fname==""} return
        if [string match -nocase *.csv "$fname"] {
            $item.table.exportToCSV $fname
        } elseif [string match -nocase *.tex "$fname"] {
            $item.table.exportToLaTeX $fname
        } else {
            switch -glob $type {
                "*(csv)" {$item.table.exportToCSV $fname.csv}
                "*(tex)" {$item.table.exportToLaTeX $fname.tex}
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

        # disable or enable the name field depending on type being selected
        bind .wiring.editVar.entry20 <<ComboboxSelected>> {
            if {[.wiring.editVar.entry20 get]=="constant"} {
                .wiring.editVar.entry10 configure -state disabled
            } else {
                .wiring.editVar.entry10 configure -state enabled
            }
        }
        
        # initialise variable type when selected from combobox
        bind .wiring.editVar.entry10 <<ComboboxSelected>> {
            getValue [valueId [.wiring.editVar.entry10 get]]
            .wiring.editVar.entry20 set [value.type]
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
            set item minsky.canvas.item
            convertVarType [$item.valueId] $editVarInput(Type)
            $item.name $editVarInput(Name)
            $item.init $editVarInput(Initial Value)
            $item.rotation  $editVarInput(Rotation)
            $item.tooltip  $editVarInput(Short description)
            $item.detailedText  $editVarInput(Detailed description)
            $item.sliderMax  $editVarInput(Slider Bounds: Max)
            $item.sliderMin  $editVarInput(Slider Bounds: Min)
            $item.sliderStep  $editVarInput(Slider Step Size)
            $item.sliderStepRel  $editVarInput(relative)
            makeVariablesConsistent
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
#  
#  proc syncVarType {} {
#              values.get $varInput(Name)
#              set varInput(Type) [value.type]
#          }
#  
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
                .wiring.initVar.entry10 configure -state enabled
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
        bind .wiring.editConstant <Key-Escape> {.wiring.editConstant.buttonBar.cancel invoke}

    } else {
        wm deiconify .wiring.editConstant
    }
}
#  
#  proc cleanEditConstantConfig {} {
#      global rowdict
#      foreach name [array names rowdict] {
#          set row $rowdict($name)
#          catch {grid remove .wiring.editConstant.label$row .wiring.editConstant.entry$row}
#      }
#  }
#  
#  proc configEditConstantForConstant {} {
#      global rowdict
#      cleanEditConstantConfig
#      set i 10
#      foreach var {
#          "Name"
#          "Value"
#          "Rotation"
#          "Slider Bounds: Max"
#          "Slider Bounds: Min"
#          "Slider Step Size"
#      } {
#          set row $rowdict($var)
#          grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
#          grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
#          incr i 10
#      }
#  }
#  
#  proc configEditConstantForIntegral {} {
#      global rowdict
#      cleanEditConstantConfig
#      set i 10
#      foreach var {
#          "Name"
#          "Value"
#          "Rotation"
#      } {
#          set row $rowdict($var)
#          grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
#          grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
#          incr i 10
#      }
#  }
#  
#  proc configEditConstantForData {} {
#      global rowdict
#      cleanEditConstantConfig
#      set i 10
#      foreach var {
#          "Name"
#          "Rotation"
#      } {
#          set row $rowdict($var)
#          grid .wiring.editConstant.label$row -row $i -column 10 -sticky e
#          grid .wiring.editConstant.entry$row -row $i -column 20 -sticky ew -columnspan 2
#          incr i 10
#      }
#  }
#  
#  
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
            closeEditWindow .wiring.editOperation
        }
        button .wiring.editOperation.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.editOperation}
        bind .wiring.editOperation <Key-Return> {invokeOKorCancel .wiring.editOperation.buttonBar}
        bind .wiring.editOperation <Key-Escape> {.wiring.editOperation.buttonBar.cancel invoke}
        pack .wiring.editOperation.buttonBar.ok [label .wiring.editOperation.buttonBar.spacer -width 2] .wiring.editOperation.buttonBar.cancel -side left -pady 10
        pack .wiring.editOperation.buttonBar -side bottom
        
        
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
    set editVarInput(title) "[$item.name]: Value=[value.value]"
    tkwait visibility .wiring.editVar
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
    getValue :$constInput(Name)
    value.init $constInput(Value)
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
 		tkwait visibility .wiring.editOperation
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
            tkwait visibility .wiring.editConstant
            grab set .wiring.editConstant
            wm transient .wiring.editConstant
        }
        "Group" {groupEdit}
        "GodleyIcon" {openGodley}
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
        minsky.canvas.item.readData $f
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
    tkwait visibility .wiring.initRandom
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
    tkwait visibility .wiring.note
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
