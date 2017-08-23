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
    -command addNewGodleyItem
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
    button .wiring.menubar.line$menubarLine.$op -image [set op]Img -command "minsky.addOperation $op" -height 24 -width 24
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
    -height 24 -width 37 -command {addNote "Enter your note here"}
tooltip .wiring.menubar.line$menubarLine.note "Note"
pack .wiring.menubar.line$menubarLine.note -side left 

# pack menubar lines
for {set i 0} {$i<=$menubarLine} {incr i} {
    pack .wiring.menubar.line$i -side top -anchor w
}
pack .wiring.menubar -fill x

image create canvasImage minskyCanvas -canvas minsky.canvas
label .wiring.canvas -image minskyCanvas -height $canvasHeight -width $canvasWidth
pack .wiring.canvas -fill both -expand 1
bind .wiring.canvas <ButtonPress-1> {minsky.canvas.mouseDown %x %y}
bind .wiring.canvas <ButtonRelease-1> {minsky.canvas.mouseUp %x %y}
bind .wiring.canvas <Motion> {minsky.canvas.mouseMove %x %y}

proc get_pointer_x {c} {
    #  return [expr {[winfo pointerx $c] - [winfo rootx $c]}]
      return [winfo pointerx $c]
}

proc get_pointer_y {c} {
    #  return [expr {[winfo pointery $c] - [winfo rooty $c]}]
    return [winfo pointery $c]
}

bind . <Key-plus> {zoom 1.1}
bind . <Key-equal> {zoom 1.1}
bind . <Key-minus> {zoom [expr 1.0/1.1]}
# mouse wheel bindings for X11
bind .wiring.canvas <Button-4> {zoomAt %x %y 1.1}
bind .wiring.canvas <Button-5> {zoomAt  %x %y [expr 1.0/1.1]}
# mouse wheel bindings for pc and aqua
bind .wiring.canvas <MouseWheel> { if {%D>=0} {zoomAt %x %y 1.1} {zoomAt  %x %y [expr 1.0/(1.1)]} }

bind .wiring.canvas <Alt-Button-1> {
    tk_messageBox -message "Mouse coordinates %x %y"
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
       
#    # sliders need to be readjusted, because zooming doesn't do the right thing
#    foreach v [wiringGroup.items.#keys] {
#        wiringGroup.item.get $v
#        if {[wiringGroup.item.visible] && [string equal -length 8 "Variable" [wiringGroup.item.classType]]} {
#            var.get $v
#            foreach item [.wiring.canvas find withtag slider$v] {
#                set coords [.wiring.canvas coords $item]
#                # should be only one of these anyway...
#                .wiring.canvas coords $item [var.x] [sliderYCoord [var.y]]
#            }
#        }
#    }
}

.menubar.ops add command -label "Godley Table" -command addNewGodleyItem

.menubar.ops add command -label "Variable" -command "addVariable" 
foreach var [availableOperations] {
    if {$var=="numOps"} break
    .menubar.ops add command -label [regsub {(.*)_$} $var {\1}] -command "minsky.addOperation $var"
}
#  
#  proc clearTempBindings {} {
#      bind .wiring.canvas <Motion> {}
#      bind .wiring.canvas <Enter> {}
#      bind . <Key-Escape> {handleEscapeKey}
#  }
 
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
        value.get [canvas.itemFocus.valueId]
        canvas.itemFocus.rotation [set varInput(Rotation)]
        canvas.itemFocus.tooltip [set "varInput(Short description)"]
        canvas.itemFocus.detailedText [set "varInput(Detailed description)"]
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

proc addConstantOrVariable {} {
    global varInput varType
    set varInput(Name) ""
    set varInput(Value) ""
    set varInput(Type) $varType
    set "varInput(Short description)" ""
    set "varInput(Detailed description)" ""
    deiconifyInitVar
    .wiring.initVar.entry10 configure -values [variableValues.#keys]
    ::tk::TabToWindow $varInput(initial_focus);
    tkwait visibility .wiring.initVar
    grab set .wiring.initVar
    wm transient .wiring.initVar
}


# add operation from a keypress
proc addOperationKey {op} {
    global globals constInput
    set id [wiringGroup.addOperation $op]
    op.get $id
    op.set
    global moveOffs$id.x moveOffs$id.y 
    set moveOffs$id.x 0
    set moveOffs$id.y 0

    newItem $id
    move $id [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
    if {$op=="constant"} {
	editItem $id
	set constInput(cancelCommand) "cancelPlaceNewOp $id;closeEditWindow .wiring.editConstant"
    }
    return $id
}

# handle arbitrary text typed into the canvas
set textBuffer ""
proc textInput {char} {
    global textBuffer globals
    #ignore anything unprintable!
    set x [get_pointer_x .wiring.canvas]
    set y [get_pointer_y .wiring.canvas]
#    if [string is print $char] {
#        if {[llength [.wiring.canvas find withtag textBuffer]]==0} {
#            .wiring.canvas create text $x $y -tags textBuffer
#        }
#        append textBuffer $char
#        .wiring.canvas itemconfigure textBuffer -text $textBuffer
#    } elseif {$char=="\r"} {
#        .wiring.canvas delete textBuffer
#        if {[lsearch [availableOperations] $textBuffer]>-1} {
#            addOperationKey $textBuffer
#        } elseif {![string match "\[%#\]*" $textBuffer]} {
#            # if no space in text, add a variable of that name
#            set id [newVariable $textBuffer "flow"]
#            var.get $id
#            var.moveTo $x $y
#            initGroupList
#            newItem $id
#        } else {
#            set id [wiringGroup.newNote]
#            wiringGroup.item.get $id
#            # trim off leading comment character
#            wiringGroup.item.detailedText [string range $textBuffer 1 end]
#            wiringGroup.item.moveTo [.wiring.canvas canvasx [get_pointer_x .wiring.canvas]]\
#                [.wiring.canvas canvasy [get_pointer_y .wiring.canvas]]
#            newItem $id
#        }
#        # TODO add arbitrary comment boxes
#        set textBuffer ""
#    }
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
    } elseif [itemsSelected] {
        cut
    } else {
        set tags [.wiring.canvas gettags  [.wiring.canvas find withtag current]]
        set re {item([0-9]+)}
        if [regexp $re [lsearch -regexp -inline $tags $re] tag id] {
            deleteItem $id $tag
        }
        set re {wire([0-9]+)}
        if [regexp $re [lsearch -regexp -inline $tags $re] tag  id] {
            deleteWire $id
            .wiring.canvas delete wire$id
        }
    }
}
#  proc redraw {id} {
#      wiringGroup.item.get $id
#      .wiring.canvas coords item$id [wiringGroup.item.x] [wiringGroup.item.y]
#      .wiring.canvas delete handles
#      wiringGroup.adjustWires $id
#  }
#  
#  set itemFocused 0
#  
#  proc updateItemPos {id} {
#      global globals
#      # ignore errors that may occur if the object vanishes before now
#      catch {redraw $id}
#      unset globals(updateItemPositionSubmitted$id)
#      doPushHistory 1
#  }    
#  
#  proc submitUpdateItemPos {id} {
#      global globals
#      if {!
#          ([info exists globals(updateItemPositionSubmitted$id)] &&
#           [set globals(updateItemPositionSubmitted$id)])} {
#          # only submitted if no update already scheduled
#          set globals(updateItemPositionSubmitted$id) 1
#          after idle updateItemPos $id
#      }
#  }
#  
#  # moveSet is used to determine the offset of where the mouse was
#  # clicked to the anchor point of the item, so that clicking on an item
#  # doesn't cause it to jump
#  proc moveSet {id x y} {
#      wiringGroup.item.get $id
#      set x [.wiring.canvas canvasx $x]
#      set y [.wiring.canvas canvasy $y]
#      global moveOffs$id.x moveOffs$id.y
#      set moveOffs$id.x [expr $x-[wiringGroup.item.x]]
#      set moveOffs$id.y [expr $y-[wiringGroup.item.y]]
#  #TODO
#  #    if {"$item"=="group"} {
#  #        initGroupList $id
#  #    } {
#  #        initGroupList
#  #    }
#  }
#  
#  proc move {id x y} {
#      pushFlags
#      doPushHistory 0
#      wiringGroup.item.get $id
#     global moveOffs$id.x moveOffs$id.y
#  # ticket #220: Windows 8 does not always generate mousedown events
#  # before sending mouse move events. This little bit of merde fakes a
#  # mousedown event if it hasn't been received yet. This at least gets
#  # rid of the error message, but the "acceleration" will probably still
#  # be there.
#      if {![info exists moveOffs$id.x] || ![info exists moveOffs$id.y]} {
#          moveSet $id $x $y
#      }
#  
#      set x [expr $x-[set moveOffs$id.x]]
#      set y [expr $y-[set moveOffs$id.y]]
#      wiringGroup.item.moveTo [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]
#      #.wiring.canvas coords item$id [wiringGroup.item.x] [wiringGroup.item.y]
#      redraw $id
#  #    if {[item.classType]=="Group"} {
#  #        # update all item positions to ensure contained items are correctly updated
#  #        foreach i [items.#keys] {
#  #            if [item.visible] {
#  #                item.get $i
#  #                .wiring.canvas coords item$i [item.x] [item.y]
#  #            }
#  #        }
#  #        item.get $id
#  #    }
#      wiringGroup.item.zoomFactor [wiringGroup.localZoomFactor $id [wiringGroup.item.x] [wiringGroup.item.y]]
#      wiringGroup.item.set $id
#      submitUpdateItemPos $id
#  #TODO
#  #    switch $item {
#  #        "var" {
#  #            foreach item [.wiring.canvas find withtag slider$id] {
#  #                set coords [.wiring.canvas coords $item]
#  #                # should be only one of these anyway...
#  #                .wiring.canvas coords $item [.wiring.canvas canvasx $x] \
#  #                    [sliderYCoord [.wiring.canvas canvasy $y]]
#  #            }
#  #        }
#  #    }
#      popFlags
#  }
#  
#  proc newItem {id} {
#      wiringGroup.item.get $id
#      .wiring.canvas create item [wiringGroup.item.x] [wiringGroup.item.y] -id $id -tags "items item$id"
#     if {[wiringGroup.item.classType]=="GodleyIcon"} {
#         .wiring.canvas bind item$id <Double-Button-1> "doubleMouseGodley $id %x %y"
#         .wiring.canvas bind item$id <Enter> "godleyToolTip $id %x %y; itemEnterLeave item $id item$id 1"
#         .wiring.canvas bind item$id <Motion> "changeToolTip $id %x %y"
#      } else {
#          .wiring.canvas bind item$id <Double-Button-1> "editItem $id"
#          .wiring.canvas bind item$id <Enter> "itemEnterLeave item $id item$id 1"
#      }
#      .wiring.canvas bind item$id <Leave> "itemEnterLeave item $id item$id 0"
#      .wiring.canvas bind item$id <Button-1> "onClick $id item$id %x %y"
#  }
#  
#  proc addNewGodleyItem {} {
#      set id [addGodleyTable]
#      global moveOffs$id.x moveOffs$id.y
#      set moveOffs$id.x 0
#      set moveOffs$id.y 0
#      
#      newItem $id
#    
#      # event bindings for initial placement
#      bind .wiring.canvas <Enter> "move $id %x %y"
#      bind .wiring.canvas <Motion> "move $id %x %y"
#      bind .wiring.canvas <Button-1> clearTempBindings
#      bind . <Key-Escape> "clearTempBindings
#         deleteGodleyTable $id
#         .wiring.canvas delete godley$id"
#  
#  }
#  
#  proc addNewGodleyItemKey {} {
#      set id [addGodleyTable]
#      global moveOffs$id.x moveOffs$id.y
#      set moveOffs$id.x 0
#      set moveOffs$id.y 0
#  
#      set id addGodleyItem
#      newItem $id
#    
#      move $id [get_pointer_x .wiring.canvas] [get_pointer_y .wiring.canvas]
#  }
#  
#  # global godley icon resource
#  setGodleyIconResource $minskyHome/icons/bank.svg
#  
#  proc godleyToolTipText {id x y} {
#      set varId [selectVar $id [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
#      if {$varId>=0} {
#          var.get $varId
#          set text [var.name]
#      } else {
#          godley.get $id
#          set text [godley.tooltip]
#          if {$text==""} {
#              set text [godley.table.title]
#          }
#      }
#      return $text
#  }
#  
#  proc godleyToolTip {id x y} {
#      if {[llength [.wiring.canvas find withtag tooltip]]==0} {
#          .wiring.canvas create text [.wiring.canvas canvasx [expr $x+20]] [.wiring.canvas canvasy [expr $y-20]] -tags "tooltip tooltiptext" -anchor w -text "[godleyToolTipText $id $x $y]"
#          .wiring.canvas create rectangle [.wiring.canvas bbox tooltiptext] -fill white -tags "tooltip tooltipBG"
#          .wiring.canvas raise tooltiptext
#      }
#  }
#  
#  proc changeToolTip {id x y} {
#      .wiring.canvas itemconfigure tooltiptext -text "[godleyToolTipText $id $x $y]"
#      .wiring.canvas coords tooltiptext [.wiring.canvas canvasx [expr $x+20]] [.wiring.canvas canvasy [expr $y-20]]
#      .wiring.canvas coords tooltipBG [.wiring.canvas bbox tooltiptext]
#  }
#  
#  
#  
#  #proc newGodleyItem {id} {
#  #    global minskyHome
#  #
#  #    godley.get $id
#  #
#  #    .wiring.canvas create godley [godley.x] [godley.y] -id $id -tags "godleys godley$id"
#  #    .wiring.canvas lower godley$id
#  #
#  #    setM1Binding godley $id godley$id
#  #    .wiring.canvas bind godley$id <<middleMouse>> \
#  #        "wires::startConnect \[closestOutPort %x %y \] godley$id %x %y"
#  #    .wiring.canvas bind godley$id <<middleMouse-Motion>> \
#  #        "wires::extendConnect \[closestOutPort %x %y \] godley$id %x %y"
#  #    .wiring.canvas bind godley$id <<middleMouse-ButtonRelease>> \
#  #        "wires::finishConnect godley$id %x %y"
#  #    .wiring.canvas bind godley$id  <Double-Button-1> "doubleMouseGodley $id %x %y"
#  #    .wiring.canvas bind godley$id <Enter> "godleyToolTip $id %x %y; itemEnterLeave godley $id godley$id 1"
#  #    .wiring.canvas bind godley$id <Leave> "itemEnterLeave godley $id godley$id 0"
#  #    .wiring.canvas bind godley$id <Motion> "changeToolTip $id %x %y"
#  #}
#  
#  proc rightMouseGodley {id x y X Y} {
#      set varId [wiringGroup.selectVar $id [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
#      if {$varId>=0} {
#          .wiring.context delete 0 end
#          .wiring.context add command -label "Edit" -command "wiringGroup.var.get $varId; editVar"
#          .wiring.context add command -label "Copy" -command "
#             item.get $varId
#             copyVar 
#             item.rotation 0
#          "
#          .wiring.context post $X $Y
#      } else {
#          contextMenu $id $X $Y
#      }
#  }
#  
#  proc doubleMouseGodley {id x y} {
#      set varId [wiringGroup.selectVar $id [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
#      if {$varId>=0} {
#          editItem $varId
#      } else {
#          openGodley $id
#      }
#  }
#  
#  #proc updateGodleyItem {id} {
#  #    .wiring.canvas delete godley$id
#  #    newItem $id
#  #}
#  
#  proc createWire {coords} {
#      return [.wiring.canvas create line $coords -tags "wires" -arrow last -smooth bezier]
#  } 
#  
#  proc newWire {wireid} {
#      wiringGroup.wire.get $wireid
#      .wiring.canvas addtag wire$wireid withtag [createWire [wiringGroup.wire.coords]]  
#      .wiring.canvas bind wire$wireid <Enter> "submitDecorateWire $wireid; set itemFocused 1"
#      .wiring.canvas bind wire$wireid <Leave> "set itemFocused 0"
#      .wiring.canvas bind wire$wireid <<contextMenu>> "wireContextMenu $wireid %X %Y"
#      # mouse-1 clicking on wire starts wiring from the from port
#  #    .wiring.canvas bind $wire <Button-1> "set clicked 1; wires::startConnect [wiringGroup.wire.from] $wire %x %y"
#  #    .wiring.canvas bind $wire <B1-Motion> "wires::extendConnect [wiringGroup.wire.from] $wire %x %y"
#  #    .wiring.canvas bind $wire <B1-ButtonRelease> "set clicked 0; wires::finishConnect $wire %x %y"
#  }
#  
#  proc updateCoords {wire handle pos x y} {
#  #    assert {[llength [.wiring.canvas find withtag wire$wire]]==1}
#      set coords [.wiring.canvas coords wire$wire]
#      set x0 [lindex $coords $pos]
#      set y0 [lindex $coords [expr $pos+1]]
#      set x [.wiring.canvas canvasx $x]
#      set y [.wiring.canvas canvasy $y]
#      .wiring.canvas move $handle [expr $x-$x0] [expr $y-$y0]
#      lset coords $pos $x
#      lset coords [expr $pos+1] $y
#      .wiring.canvas coords wire$wire $coords
#      wire.get $wire
#      wire.coords $coords
#      wire.set
#  }
#  
#  proc insertCoords {wire handle pos x y} {
#      global handles
#  #    assert {[llength [.wiring.canvas find withtag wire$wire]]==1}
#      if {![info exists handles($handle)]} {
#          set handles($handle) 1
#          # add current handle coordinates to the wire shape
#          set coords [.wiring.canvas coords wire$wire]
#          set coords [linsert $coords $pos [.wiring.canvas canvasx $x] \
#                      [.wiring.canvas canvasy $y]]
#          .wiring.canvas coords wire$wire $coords
#      }
#      updateCoords $wire $handle $pos $x $y
#  }
#  
#  proc createHandle {w x y} {
#      return [
#  # attach wire information, allowing a wire context menu to be posted on right mouse click
#          .wiring.canvas create oval \
#          [expr $x-3] [expr $y-3] [expr $x+3] [expr $y+3] \
#          -fill blue  -tag "wires wire$w handle$w handles"
#      ]
#  }
#  
#  proc deleteHandle {wire handle pos} {
#      .wiring.canvas delete handles
#      set coords [lreplace [.wiring.canvas coords wire$wire] $pos [expr $pos+1]]
#      .wiring.canvas coords wire$wire $coords        
#      wire.get $wire
#      wire.coords $coords
#      submitDecorateWire $wire
#  }
#      
#  proc createHandleInBetween {wire pos coords} {
#      set h [
#             createHandle $wire [
#                                 expr ([lindex $coords $pos]+[lindex $coords [expr $pos+2]])/2
#                                ] [
#                                   expr ([lindex $coords [expr $pos+1]]+\
#                                             [lindex $coords [expr $pos+3]])/2]
#            ]
#      .wiring.canvas bind $h <B1-Motion> \
#          "insertCoords $wire $h [expr $pos+2] %x %y" 
#      .wiring.canvas bind $h <ButtonRelease-1> \
#          ".wiring.canvas delete handles"        
#  }
#  
#  
#  proc decorateWire {wire} {
#      # no need to redecorate if wire is already done
#      if {[llength [.wiring.canvas find withtag handle$wire]]==0} {
#          .wiring.canvas delete handles
#          global handles
#          set coords [.wiring.canvas coords wire$wire]
#          for {set i 0} {$i<[llength $coords]-2} {incr i 2} {
#              if {$i>0} {
#                  set h [
#                         createHandle $wire \
#                             [lindex $coords $i] [lindex $coords [expr $i+1]]
#                        ]
#                  set handle($h) 1
#                  .wiring.canvas bind $h <B1-Motion> \
#                      "updateCoords $wire $h $i %x %y"
#                  .wiring.canvas bind $h <Double-Button-1> \
#                      "deleteHandle $wire $h $i"
#                  .wiring.canvas bind $h <ButtonRelease-1> \
#                      "after 200 {.wiring.canvas delete handles}"
#              }
#              createHandleInBetween $wire $i $coords
#          }
#      }
#      global globals
#      set globals(decorateWireSubmitted) 0
#  }
#  
#  proc submitDecorateWire {wire} {
#      global globals
#      if {!([info exists globals(decorateWireSubmitted)] &&
#          [set globals(decorateWireSubmitted)])} {
#          set globals(decorateWireSubmitted) 1
#          after idle decorateWire $wire
#      }
#  }
#  
#  namespace eval wires {
#      proc startConnect {id x y} {
#          set x [.wiring.canvas canvasx $x]
#          set y [.wiring.canvas canvasy $y]
#          namespace children
#          if {![namespace exists $id]} {
#              namespace eval $id {
#                  variable wire
#                  variable fromId
#                  variable x0
#                  variable y0 
#                 
#                  proc extendConnect {x y} {
#                      variable wire
#                      variable x0
#                      variable y0 
#                      .wiring.canvas coords $wire $x0 $y0 $x $y
#                  }
#                  
#                  proc finishConnect {x y} {
#                      set x [.wiring.canvas canvasx $x]
#                      set y [.wiring.canvas canvasy $y]
#                      variable wire
#                      variable fromId
#                      variable x0
#                      variable y0 
#                      set wireid [wiringGroup.addWire $fromId $x0 $y0 $x $y [.wiring.canvas coords $wire]]
#                      .wiring.canvas delete $wire
#                      if {$wireid >=0} {
#                          newWire $wireid
#                      }
#                      namespace delete [namespace current]
#                  }
#  
#              }
#  
#              set [set id]::wire [createWire "$x $y $x $y"]
#              set [set id]::fromId $id
#              set [set id]::x0 $x
#              set [set id]::y0 $y
#          }
#      }        
#  
#      proc extendConnect {id x y} {
#          if [namespace exists $id] {
#              set x [.wiring.canvas canvasx $x]
#              set y [.wiring.canvas canvasy $y]
#              [set id]::extendConnect $x $y
#          }
#      }
#  
#      proc finishConnect {id x y} {[set id]::finishConnect $x $y}
#  }
#  
#  ## adjust the begin or end of line when a port moves
#  #proc adjustWire {portId} {
#  #    foreach w [wiresAttachedToPort $portId] {
#  #        .wiring.canvas delete handles
#  #        wire.get $w
#  #        if [wire.visible] {
#  #            # ensure wire exists on canvas
#  #            if {[.wiring.canvas type wire$w]==""} {
#  #                newWire [createWire [wire.coords]] $w
#  #            }
#  #            eval .wiring.canvas coords wire$w [wire.coords]
#  #        }
#  #   }
#  #}
#  
#  proc straightenWire {id} {
#      wire.get $id
#      wire.straighten
#      eval .wiring.canvas coords wire$id [wire.coords]
#  }
#  
#  # a closestOutPort that takes into account panning
#  proc closestOutPort {x y} {
#      return [wiringGroup.closestOutPort [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
#  }
#  
#  # track button state
#  set clicked 0
#  proc unbindOnRelease {tag} {
#      global clicked
#      set clicked 0
#      .wiring.canvas bind $tag <B1-Motion> ""
#      .wiring.canvas bind $tag <B1-ButtonRelease> ""
#  }
#  
#  # called when clicking on an item 
#  proc onClick {id tag x y} {
#      global clicked
#      set clicked 1
#      wiringGroup.item.get $id
#      switch [wiringGroup.item.clickType [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]] {
#          "onPort" {
#              wires::startConnect $id $x $y
#              .wiring.canvas bind $tag <B1-Motion> \
#                  "wires::extendConnect $id %x %y"
#              .wiring.canvas bind $tag <B1-ButtonRelease>  \
#                  "wires::finishConnect $id %x %y; unbindOnRelease $tag"
#          }
#          "onItem" {
#              moveSet $id $x $y
#              .wiring.canvas bind $tag <B1-Motion> "move $id %x %y"
#              .wiring.canvas bind $tag <B1-ButtonRelease> "move $id %x %y; wiringGroup.checkAddGroup $id %x %y; unbindOnRelease $tag"
#          }
#          "outside" {
#              .wiring.canvas bind $tag <B1-Motion> "lasso %x %y"
#              .wiring.canvas bind $tag <B1-ButtonRelease> "lassoEnd %x %y; unbindOnRelease $tag"
#          }
#      }
#  }
#  
#  proc setM1Binding {id tag} {
#      .wiring.canvas bind $tag <Button-1> "onClick $id $tag %x %y"
#  }
#  
#  # lasso mode when not clicked on an icon       
#  bind .wiring.canvas <Button-1> {if {!$itemFocused} {lasso %x %y}}
#  bind .wiring.canvas <B1-Motion> {if {!$itemFocused && !$clicked} {lasso %x %y}}
#  bind .wiring.canvas <B1-ButtonRelease> {if {!$itemFocused && !$clicked} {lassoEnd %x %y}}
#  
# pan mode
bind .wiring.canvas <Shift-Button-1> {set panOffsX [expr %x-[model.x]]; set panOffsY [expr %y-[model.y]]}
bind .wiring.canvas <Shift-B1-Motion> {model.moveTo [expr %x-$panOffsX] [expr %y-$panOffsY]; canvas.requestRedraw}
#  
#  proc recentreCanvas {} {
#      .wiring.canvas xview moveto 0.5
#      .wiring.canvas yview moveto 0.5
#      .equations.canvas xview moveto 0.5
#      .equations.canvas yview moveto 0.5
#  }
#  
#  proc anyItems {tag} {
#      return [expr [llength [.wiring.canvas find withtag $tag]]>0]
#  }
#  
#  proc accessed {items item id} {
#      catch {if {$item.id==$id} {return 1}}
#      return [wiringGroup.$items.hasBeenAccessed $id]
#  }
#  
#  proc delIfAccessed {items item id} {
#      if [accessed $items $item $id] {.wiring.canvas delete $item$id}
#  }
#  
menu .wiring.context -tearoff 0
#  
#  proc toggleCoupled {id} {
#      integral.get $id
#      integral.toggleCoupled
#  }
#  
#  proc addIntegral name {
#      set id [addOperation integrate]
#      integral.get $id
#      integral.description $name
#  }
#  
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
#  
#  proc saveSelection {} {
#      global workDir
#      set fname [tk_getSaveFile -defaultextension .mky -initialdir $workDir]
#      if [string length $fname] {
#          saveSelectionAsFile $fname
#      }
#  }
#  
#  proc canvasHelp {x y} {
#      set itemlist [.wiring.canvas find withtag current]
#      if {[llength $itemlist]==0} {
#          help DesignCanvas
#      } else {
#          # should only be one or zero current items?
#          set canvId [lindex $itemlist 0]
#          # assuming that the one true tag is of the form aaaNNN - a is lowercase letter, N a digit
#          set tags [.wiring.canvas gettags $canvId]
#          set tag [lindex $tags [lsearch -regexp $tags {[a-z]+[0-9]+}]]
#          set item [regsub {[0-9]+} $tag ""]
#          # extract the item id from its tag
#          set id [regsub {[a-z]+} $tag ""]
#          wiringGroup.$item.get $id
#  
#          switch $item {
#              var {help Variable}
#              op {help "op:[op.name]"}
#              godley {help GodleyTable}
#              group {help Group}
#              plot {help Plot}
#          }
#      }
#  }
#  


bind .wiring.canvas <Double-Button-1> {
    if [getItemAt %x %y] {
        editItem
    }
}

bind .wiring.canvas <<contextMenu>> {
    if [getItemAt %x %y] {
        contextMenu %x %y %X %Y
    } elseif [getWireAt %x %y] {
        wireContextMenu %x %y
    } else {
        canvasContext  %X %Y
    }

#    set items [.wiring.canvas find withtag current]
#    if {[llength $items]==0} {
#        canvasContext %X %Y
#    } else {
#        foreach item $items {
#            if {[.wiring.canvas type $item]=="item"} {
#                # TODO - this is so kludgy
#                set tags [.wiring.canvas gettags $item]
#                set tag [lindex $tags [lsearch -regexp $tags {item[0-9]+}]]
#                set id [string range $tag 4 end]
#                wiringGroup.item.get $id
#                switch [wiringGroup.item.classType] {
#                    "GodleyIcon" "rightMouseGodley $id %x %y %X %Y"
#                    "Group" "rightMouseGroup $id %x %y %X %Y"
#                    default "contextMenu $id %X %Y"
#                }
#            }
#        }
#    }
}

#  proc raiseItem {item} {
#      .wiring.canvas raise $item all
#  }
#  proc lowerItem {item} {
#      .wiring.canvas lower $item all
#  }
#  
proc wireContextMenu {id x y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help Wires}
    .wiring.context add command -label Description -command "postNote wire $id"
    .wiring.context add command -label "Straighten" -command "wire.straighten; canvas.redraw"
#    .wiring.context add command -label "Raise" -command "raiseItem wire$id"
#    .wiring.context add command -label "Lower" -command "lowerItem wire$id"
    .wiring.context add command -label "Browse object" -command "obj_browser canvas.wire.*"
    .wiring.context add command -label "Delete wire" -command "deleteWire; canvas.redraw"
    tk_popup .wiring.context $x $y
}

proc findDefinition {} {
    if [canvas.findVariableDefinition] {
        canvas.indicateItem
    } else {
        tk_messageBox -message "Definition not found"
    }
}
#  
# context menu
proc contextMenu {x y X Y} {
    set item minsky.canvas.item
    .wiring.context delete 0 end
    # find out what type of item we're referring to
    switch -regex [$item.classType] {
        "Variable*" {
            .wiring.context add command -label Help -command {help Variable}
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Value [minsky.canvas.item.value]" 
            .wiring.context add command -label "Find definition" -command "findDefinition"
            .wiring.context add command -label "Edit" -command "editItem"
#            .wiring.context add checkbutton -label "Slider" \
#                -command "drawSlider $id $x $y" \
#                -variable "sliderCheck$id"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            if {[$item.type]=="flow" && ![inputWired [$item.valueId]]} {
                .wiring.context add command -label "Add integral" -command "addIntegral [$item.name]"
            }
            .wiring.context add command -label "Flip" -command "item.flip; flip_default"
        }
        "Operation*|IntOp" {
            .wiring.context add command -label Help -command "help op:[$item.name]"
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Port values [$item.portValues]" 
            .wiring.context add command -label "Edit" -command "editItem"             
            if {[$item.name]=="data"} {
               .wiring.context add command -label "Import Data" \
                    -command "importData $id" 
            }
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            .wiring.context add command -label "Flip" -command "minsky.canvas.item.flip; flip_default"
            if {[$item.name]=="integrate"} {
                .wiring.context add command -label "Toggle var binding" -command "minsky.canvas.item.toggleCoupled; canvas.requestRedraw"
            }
        }
        "PlotWidget" {
            .wiring.context add command -label Help -command {help Plot}
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Expand" -command "plotDoubleClick [TCLItem]"
            .wiring.context add command -label "Make Group Plot" -command "canvas.item.makeDisplayPlot"
            .wiring.context add command -label "Resize" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Options" -command "doPlotOptions $item"
        }
        "GodleyIcon" {
            .wiring.context add command -label Help -command {help GodleyTable}
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Open Godley Table" -command "openGodley"
            .wiring.context add command -label "Resize Godley" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Export to file" -command "godley::export"
        }
        "Group" {
            .wiring.context add command -label Help -command {help Group}
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Edit" -command "groupEdit"
            .wiring.context add command -label "Open in canvas" -command "openGroupInCanvas"
            .wiring.context add command -label "Zoom to display" -command "group::zoomToDisplay"
            .wiring.context add command -label "Remove plot icon" -command "item.removeDisplayPlot"
            .wiring.context add command -label "Resize" -command "canvas.lassoMode itemResize"
            .wiring.context add command -label "Copy" -command "canvas.copyItem"
            .wiring.context add command -label "Save group as" -command "group::save"
            .wiring.context add command -label "Flip" -command "minsky.canvas.item.flip; flip_default"
            .wiring.context add command -label "Flip Contents" -command "group::flipContents"
            .wiring.context add command -label "Ungroup" -command "ungroupGroupItem"
        }
        "Item" {
            .wiring.context delete 0 end
            .wiring.context add command -label Help -command {help Notes}
            .wiring.context add command -label Edit -command "postNote item"
        }
        SwitchIcon {
            .wiring.context add command -label Help -command {help Switches}
            .wiring.context add command -label Description -command "postNote item"
            .wiring.context add command -label "Add case" -command "incrCase 1" 
            .wiring.context add command -label "Delete case" -command "incrCase -1" 
            .wiring.context add command -label "Flip" -command "minsky.canvas.item.flipped [expr ![minsky.canvas.item.flipped]]; canvas.requestRedraw"
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
#  
#  proc deleteItem {id tag} {
#      .wiring.canvas delete $tag
#      wiringGroup.deleteItem $id
#      .wiring.canvas delete wires
#      wiringGroup.buildMaps
#      updateCanvas
#  }
#  
#  proc copyVar {} {
#      global globals
#      set newId [wiringGroup.copyItem]
#      var.get $newId
#      newItem $newId
#      placeNewVar $newId
#  }
#  
#  proc copyOp  {id} {
#      global globals
#      item.get $id
#      set newId [copyItem]
#      op.get $newId
#      placeNewOp $newId 
#  }
#  
#  proc rotateOp {id angle} {
#      op.get $id
#      op.rotation [expr [op.rotation]+$angle]
#      op.set
#      drawOperation $id
#      wiringGroup.adjustWires $id
#  }
#  
#  proc rotateVar {id angle} {
#      var.get $id
#      var.rotation [expr [var.rotation]+$angle]
#      wiringGroup.adjustWires $id
#  }
#  
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
            value.get [valueId [.wiring.editVar.entry10 get]]
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
# TODO            setSliderProperties
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
            value.get [.wiring.initVar.entry10 get]
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
#          
#  # set attribute, and commit to original item
#  proc setItem {modelCmd attr dialogCmd} {
#      global constInput varInput editVarInput opInput
#      $modelCmd.$attr [string trim [eval $dialogCmd]]
#  }
#  
#  #proc setInitVal {var dialogCmd} {
#  #    value.get $var
#  #    value.init [eval $dialogCmd]
#  #    value.set $var
#  #}
#  
proc closeEditWindow {window} {
    grab release $window
    destroy $window
}
#  
#  proc setConstantValue {} {
#      global constInput
#      constant.value "$constInput(Value)"
#      constant.description "$constInput(Name)"
#      constant.sliderStepRel "$constInput(relative)"
#      constant.sliderMin "$constInput(Slider Bounds: Min)"
#      constant.sliderMax "$constInput(Slider Bounds: Max)"
#      constant.sliderStep "$constInput(Slider Step Size)"
#      constant.sliderBoundsSet 1
#  }
#  
#  proc setDataValue {} {
#      global constInput
#      data.description "$constInput(Name)"
#  }
#  
#  proc setIntegralIValue {} {
#      global constInput
#      integral.description "$constInput(Name)"
#      integral.getIntVar
#      value.get [item.valueId]
#      setItem value init {set constInput(Value)}
#  }
#  
proc editVar {} {
    global editVarInput
    set item minsky.canvas.item
    value.get [$item.valueId]
    deiconifyEditVar
    wm title .wiring.editVar "Edit [$item.name]"
    # populate combobox with existing variable names
    .wiring.editVar.entry10 configure -values [variableValues.#keys]

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

proc editItem {} {
    global constInput varInput editVarInput opInput
    switch -regexp [minsky.canvas.item.classType] {
        "Variable*" {editVar}
        "Operation*" {
            set opType [minsky.canvas.item.name]
            if {$opType=="integrate" || $opType=="data"} {
                set constInput(Value) ""
                set "constInput(Slider Bounds: Min)" ""
                set "constInput(Slider Bounds: Max)" ""
                set "constInput(Slider Step Size)" ""
                deiconifyEditConstant
                switch $opType {
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
                set constInput(Rotation) [minsky.canvas.item.rotation]
                set constInput(command) "
                         $setValue
                         minsky.canvas.item.rotation {set constInput(Rotation)}
                         closeEditWindow .wiring.editConstant
                     "
 		set constInput(cancelCommand) "closeEditWindow .wiring.editConstant"
                
 		::tk::TabToWindow $constInput(initial_focus);
 		tkwait visibility .wiring.editConstant
 		grab set .wiring.editConstant
 		wm transient .wiring.editConstant
                
            } else {
                set opInput(title) [minsky.canvas.item.name]
                set opInput(Rotation) [minsky.canvas.item.rotation]
                deiconifyEditOperation
 		::tk::TabToWindow $opInput(initial_focus);
 		tkwait visibility .wiring.editOperation
 		grab set .wiring.editOperation
 		wm transient .wiring.editOperation
            }
        }
        "IntOp" {
            set constInput(Value) ""
            set "constInput(Slider Bounds: Min)" ""
            set "constInput(Slider Bounds: Max)" ""
            set "constInput(Slider Step Size)" ""
            deiconifyEditConstant
            wm title .wiring.editConstant "Edit Integral"
            integral.get $id
            set constInput(ValueLabel) "Initial Value"
            integral.getIntVar
            value.get [item.valueId]
            set constInput(Value) [value.init]
            set setValue setIntegralIValue
            set constInput(Name) [integral.description]
            configEditConstantForIntegral
            set constInput(title) $constInput(Name)
            set constInput(Rotation) [integral.rotation]
            # value needs to be regotten, as var name may have changed
            set constInput(command) "
                        $setValue
                        integral.getIntVar
                        setSliderProperties
                        setItem integral rotation {set constInput(Rotation)}
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
    }
}
#  
#  proc setVarVal {v x} {
#      var.get $v
#      pushFlags
#      var.sliderSet $x
#      popFlags
#  }
#  
#  proc setSliderProperties {} {
#      if {[llength [info command wiringGroup.var.id]] && [winfo  exists .wiring.slider[wiringGroup.var.id]]} {
#          wiringGroup.var.initSliderBounds
#          if [var.sliderStepRel] {
#              set res [expr [wiringGroup.var.sliderStep]*([wiringGroup.var.sliderMax]-[wiringGroup.var.sliderMin])]
#          } else {
#              set res [wiringGroup.var.sliderStep]
#          }
#          
#          # ensure resolution is accurate enough to not mutate variable value
#          set decPos [string first . [var.value]]
#          if {$decPos==-1} {
#              set newRes 1
#          } else {
#              set newRes [expr pow(10,1+$decPos-[string len [var.value]])]
#          }
#         if {$newRes<$res} {set res $newRes}
#  
#          # ensure slider does not override value
#          wiringGroup.var.adjustSliderBounds
#  
#          .wiring.slider[wiringGroup.var.id] configure -to [wiringGroup.var.sliderMax] \
#              -from [wiringGroup.var.sliderMin] -resolution $res
#          if [catch .wiring.slider$id set [wiringGroup.var.init]] {
#              .wiring.slider$id set [wiringGroup.var.value]
#          }
#      }
#  }
#  
#  # if y is the y-coordinate of the constant, then return a y-coordinate
#  # for an attached slider
#  proc sliderYCoord {y} {
#      return [expr $y-15-10*[zoomFactor]]
#  }
#  
#  proc drawSlider {var x y} {
#      global sliderCheck$var
#      var.get $var
#      if {![info exists sliderCheck$var]} {
#          # sliderCheck$vae gets initialised to constant.sliderVisible,
#          # otherwise sliderCheck$var is more up to date
#          set sliderCheck$var [var.sliderVisible]
#      }
#      if {[var.sliderVisible]!=[set sliderCheck$var]} {
#          var.sliderVisible [set sliderCheck$var]
#      }
#  
#      if {[set sliderCheck$var]} {
#          if {![winfo exists .wiring.slider$var]} {
#              scale .wiring.slider$var -orient horizontal -width 7 -length 50 \
#                  -showvalue 1 -sliderlength 30 
#          }
#  
#          setSliderProperties
#  
#          # configure command after slider initially set to prevent
#          # constant value being set to initial state of slider when
#          # constructed.
#  #        .wiring.slider$var configure -command "setVarVal $var"
#          .wiring.slider$var configure -variable "sliderVal[var.fqName]" -command "setVarVal $var"
#  
#          #.wiring.canvas bind .wiring.slider$op Keypress-Right 
#          bind .wiring.slider$var <Enter> "focus .wiring.slider$var"
#          # add shift arrow presses as synonyms for control arrows, as
#          # control arrows are interpreted by some window managers
#          bind .wiring.slider$var <Shift-KeyPress-Left> \
#              "event generate .wiring.slider$var <Control-KeyPress-Left>"
#          bind .wiring.slider$var <Shift-KeyPress-Right> \
#              "event generate .wiring.slider$var <Control-KeyPress-Right>"
#          bind .wiring.slider$var <Shift-KeyPress-Up> \
#              "event generate .wiring.slider$var <Control-KeyPress-Up>"
#          bind .wiring.slider$var <Shift-KeyPress-Down> \
#              "event generate .wiring.slider$var <Control-KeyPress-Down>"
#          bind .wiring.slider$var <Leave> {focus .}
#  
#          .wiring.canvas create window [var.x] [sliderYCoord [var.y]] -window .wiring.slider$var -tag slider$var
#          # this is needed to ensure the setVarVal is fired _before_
#          # moving on to processing the next operation in updateCanvas
#          update
#      } else {
#          #remove any slider
#          .wiring.canvas delete slider$var
#      }
#  }
#  
#  proc importData {id} {
#      global workDir
#      data.get $id
#      set f [tk_getOpenFile -multiple 1 -initialdir $workDir]
#      if [string length $f] {
#          data.readData $f
#          updateCanvas
#      }
#  }
#  
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
#  
#  proc placeNewNote {} {
#      set id [wiringGroup.newNote]
#      wiringGroup.item.get $id
#      wiringGroup.item.detailedText "Enter your note here"
#      newItem $id
#      global moveOffs$id.x moveOffs$id.y
#      set moveOffs$id.x 0
#      set moveOffs$id.y 0
#      bind .wiring.canvas <Enter> "move $id %x %y"
#      bind .wiring.canvas <Motion> "move $id %x %y"
#      bind .wiring.canvas <Button-1> clearTempBindings
#      bind . <Key-Escape> "clearTempBindings
#         deleteNote $id
#         .wiring.canvas delete item$id"
#  
#  }
#  
#  proc findItemWithDetailedText desc {
#      foreach i [items.#keys] {
#          item.get $i
#          if {[item.detailedText]==$desc} {return $i}
#      }
#  }
#  

proc tout {args} {
  puts "$args"
}

#  # example debugging trace statements
#  #trace add execution placeNewVar enterstep tout
#  #trace add execution move enterstep tout
#  
