#  @copyright Steve Keen 2015
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

# Group (or block) functionality

# convert degrees to radian
proc radian {deg} {
    return [expr $deg*3.1415927/180]
}

setGroupIconResource $minskyHome/icons/group.svg

proc newSwitch {id} {
    global minskyHome
    switchItem.get $id
    .wiring.canvas create switch [switchItem.x] [switchItem.y] -id $id  -tags "switchItem$id switches"
    .wiring.canvas lower switchItem$id

    .wiring.canvas bind switchItem$id <Button-1> "onClick switchItem $id switchItem$id %x %y"
    .wiring.canvas bind switchItem$id <<middleMouse-Motion>> \
        "wires::extendConnect \[closestOutPort %x %y \] group$id %x %y"
    .wiring.canvas bind switchItemgroup$id <<middleMouse-ButtonRelease>> \
        "wires::finishConnect group$id %x %y"
    .wiring.canvas bind group$id  <<contextMenu>> "rightMouseSwitch $id %x %y %X %Y"
    .wiring.canvas bind switchItem$id  <Double-Button-1> "switchEdit $id"
    .wiring.canvas bind switchItem$id <Enter> "itemEnterLeave switchItem $id switchItem$id 1"
    .wiring.canvas bind switchItem$id <Leave> "itemEnterLeave switchItem $id switchItem$id 0"

    
}

proc placeNewSwitch {} {
    set id [minsky.newSwitch]
    switchItem.get $id
    switchItem.detailedText "Enter your note here"
    newSwitch $id
    global moveOffsswitchItem$id.x moveOffsswitchItem$id.y
    set moveOffsswitchItem$id.x 0
    set moveOffsswitchItem$id.y 0
    bind .wiring.canvas <Enter> "move switchItem $id %x %y"
    bind .wiring.canvas <Motion> "move switchItem $id %x %y"
    bind .wiring.canvas <Button-1> clearTempBindings
    bind . <Key-Escape> "clearTempBindings
       deleteSwitch $id
       .wiring.canvas delete switchItem$id"

}

proc rightMouseSwitch {id x y X Y} {
    group.get $id
    set var [group.selectVariable [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
    if {$var==-1} {
        contextMenu group$id $X $Y
    } else {
        .wiring.context delete 0 end
        .wiring.context add command -label "Edit" -command "editItem $var var"
        var.get $var
        .wiring.context add command -label "Copy" -command "
           copyVar $var
           var.rotation 0
           var.set
        "
        .wiring.context add command -label "Remove" -command "
           group.removeVariableById $var
           group.set
        "
        .wiring.context post $X $Y

    }
}


proc deleteSwitchItem {id} {
    deleteSwitch $id
    updateCanvas
}

proc incrCase {id delta} {
    switchItem.get $id
    switchItem.setNumCases [expr [switchItem.numCases]+$delta]
    redraw switchItem$id
}

#proc groupContext {id x y} {
#    .wiring.context delete 0 end
#    .wiring.context add command -label Help -command {help Group}
#    .wiring.context add command -label Description -command "postNote group $id"
#    .wiring.context add command -label "Edit" -command "groupEdit $id"
#    .wiring.context add command -label "Zoom to display" -command "group::zoomToDisplay $id"
#    .wiring.context add command -label "Resize" -command "group::resize $id"
#    .wiring.context add command -label "Copy" -command "group::copy $id"
#    .wiring.context add command -label "Save group as" -command "group::save $id"
#    .wiring.context add command -label "Flip" -command "group::flip $id"
#    .wiring.context add command -label "Flip Contents" -command "group::flipContents $id"
#    .wiring.context add command -label "Browse object" -command "obj_browser [eval minsky.groupItems.@elem $id].*"
#    .wiring.context add command -label "Group" -command "minsky.createGroup; updateCanvas"
#    .wiring.context add command -label "Ungroup" -command "ungroupGroupItem $id"
#    .wiring.context add command -label "Raise" -command "raiseItem group$id"
#    .wiring.context add command -label "Lower" -command "lowerItem group$id"
#    .wiring.context add command -label "Delete" -command "deleteGroupItem $id"
#    .wiring.context add command -label "content bounds" -command "
#      group.get $id
#      .wiring.canvas create rectangle \[group.cBounds\]
#     "
#}
#
#proc deiconifyEditGroup {} {
#    if {![winfo exists .wiring.editGroup]} {
#        toplevel .wiring.editGroup
#        wm title .wiring.editGroup "Edit Group"
#        wm transient .wiring.editGroup .wiring
#
#        frame .wiring.editGroup.name
#        label .wiring.editGroup.name.label -text "Name"
#        entry  .wiring.editGroup.name.val -width 20
#        pack .wiring.editGroup.name.label .wiring.editGroup.name.val -side left
#
#        frame .wiring.editGroup.rot
#        label .wiring.editGroup.rot.label -text "     Rotation"
#        entry  .wiring.editGroup.rot.val -width 20
#        pack .wiring.editGroup.rot.label .wiring.editGroup.rot.val -side left
#
#        pack .wiring.editGroup.name .wiring.editGroup.rot
#        
#        frame .wiring.editGroup.buttonBar
#        button .wiring.editGroup.buttonBar.ok -text OK
#        button .wiring.editGroup.buttonBar.cancel -text Cancel -command {
#            closeEditWindow .wiring.editGroup}
#        pack .wiring.editGroup.buttonBar.ok .wiring.editGroup.buttonBar.cancel -side left
#        pack .wiring.editGroup.buttonBar -side bottom
#        
#        bind .wiring.editGroup <Key-Return> {invokeOKorCancel .wiring.editGroup.buttonBar}
#    } else {
#        wm deiconify .wiring.editGroup
#    }
#}
#
#proc groupEdit {id} {
#    group.get $id
#    deiconifyEditGroup
#    .wiring.editGroup.name.val delete 0 end
#    .wiring.editGroup.name.val insert 0 [group.name]
#    .wiring.editGroup.rot.val delete 0 end
#    .wiring.editGroup.rot.val insert 0 [group.rotation]
#    .wiring.editGroup.buttonBar.ok configure \
#        -command {
#            group.setName [.wiring.editGroup.name.val get]
#            group.rotate [expr [.wiring.editGroup.rot.val get]-[group.rotation]]
#            group.updatePortLocation
#            group.set
#            closeEditWindow .wiring.editGroup
#        }
#    grab .wiring.editGroup
#}
#
#proc checkAddGroup {item id x y} {
#    set gid [groupTest.containingGroup [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
#    $item.get $id
#    # check for moves within group
#    if {[llength [info commands minsky.$item.group]]==0 || $gid==[$item.group]} {
#        return
#    }
#    if {$gid>=0} {
#        group.get $gid
#        if {![group.displayContents]} {.wiring.canvas delete $item$id}
#        switch $item {
#            "var" {addVariableToGroup $gid $id 1; .wiring.canvas delete $item$id}
#            "op" {addOperationToGroup $gid $id; .wiring.canvas delete $item$id}
#            "group" {
#                if [addGroupToGroup $gid $id] {
#                    .wiring.canvas delete $item$id
#                }
#            }
#        }
#        .wiring.canvas delete $item$id
#        # redraw group
#        .wiring.canvas delete group$gid groupitems$gid
#        newGroupItem $gid
#        submitUpdateItemPos group $gid
#    } else {
#        # check if it needs to be removed from a group
#        $item.get $id
#        set gid [$item.group]
#        if {$gid>=0} {
#            switch $item {
#                "var" {removeVariableFromGroup $gid $id}
#                "op" {removeOperationFromGroup $gid $id}
#                "group" {removeGroupFromGroup $gid $id}
#            }
#            # redraw group
#            .wiring.canvas dtag $item$id groupitems$gid
#            .wiring.canvas delete group$gid groupitems$gid
#            newGroupItem $gid
#        }
#    }
#    update
#}
#
#namespace eval group {
#    proc resize {id} {
#        group.get $id
#        set bbox [.wiring.canvas bbox group$id]
#        variable orig_width [expr [lindex $bbox 2]-[lindex $bbox 0]]
#        variable orig_height [expr [lindex $bbox 3]-[lindex $bbox 1]]
#        variable orig_x [group.x]
#        variable orig_y [group.y]
#        set item [eval .wiring.canvas create rectangle $bbox -tags resizeBBox]
#        # disable lasso mode
#        bind .wiring.canvas <Button-1> ""
#        bind .wiring.canvas <B1-Motion> ""
#        bind .wiring.canvas <B1-ButtonRelease> ""
#        bind .wiring.canvas <Motion> "group::resizeRect $item %x %y"
#        bind .wiring.canvas <ButtonRelease> "group::resizeItem $item $id %x %y"
#    }
#
#    # resize the bounding box to indicate how big we want the icon to be
#    proc resizeRect {item x y} {
#        set x [.wiring.canvas canvasx $x]
#        set y [.wiring.canvas canvasy $y]
#        variable orig_x
#        variable orig_y
#        set w [expr abs($x-$orig_x)]
#        set h [expr abs($y-$orig_y)]
#        .wiring.canvas coords $item  [expr $orig_x-$w] [expr $orig_y-$h] \
#            [expr $orig_x+$w] [expr $orig_y+$h]
#    }
#
#    # compute width and height and redraw item
#    proc resizeItem {item id x y} {
#        set x [.wiring.canvas canvasx $x]
#        set y [.wiring.canvas canvasy $y]
#        .wiring.canvas delete $item
#        variable orig_width
#        variable orig_height
#        variable orig_x
#        variable orig_y
#        set scalex [expr 2*abs($x-$orig_x)/double($orig_width)]
#        set scaley [expr 2*abs($y-$orig_y)/double($orig_height)]
#        # compute rotated scale factors
#        set angle [radian [group.rotation]]
#        set rx [expr $scalex*cos($angle)-$scaley*sin($angle)]
#        set ry [expr $scalex*sin($angle)+$scaley*cos($angle)]
#        group.width [expr abs($rx*[group.width])]
#        group.height [expr abs($ry*[group.height])]
#        group.computeDisplayZoom
#        group.set
#        .wiring.canvas delete group$id
#        newGroupItem $id
#        foreach p [group.ports]  {
#            adjustWire $p
#        }
#        bind .wiring.canvas <Motion> {}
#        bind .wiring.canvas <ButtonRelease> {}
#        setInteractionMode
#    }
#
#    proc copy {id} {insertNewGroup [copyGroup $id]}
#
#    proc save {id} {
#        global workDir
#        set fname [tk_getSaveFile -defaultextension .mky -initialdir $workDir]
#        if [string length $fname] {
#            saveGroupAsFile $id $fname
#        }
#    }
#
#    proc flip {id} {
#        group.get $id
#        group.rotation [expr [group.rotation]+180]
#        group.updatePortLocation
#        group.set
#        .wiring.canvas delete group$id
#        newGroupItem $id
#        foreach p [group.ports] {
#            adjustWire $p
#        }
#    }
#
#    proc flipContents {id} {
#        group.get $id
#        group.flipContents
#        group.set
#        # a bit kludgy to do a full redraw here...
#        updateCanvas
#    }
#
#    proc zoomToDisplay {id} {
#        group.get $id
#        set factor [expr 1.1*[group.computeDisplayZoom]/[group.zoomFactor]]
#        minsky.zoom [group.x] [group.y] $factor
#        .wiring.canvas scale all [group.x] [group.y] $factor $factor
#        # oh, why???
#        updateCanvas
#    }
#
#    proc button1 {id x y} {
#        group.get $id
#        if {[group.displayContents] &&
#                [group.clickType [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]!="onPort"} {
#            # use lasso mode when zoomed in
#            lasso $x $y
#            .wiring.canvas bind group$id <B1-Motion> "lasso %x %y"
#            .wiring.canvas bind group$id <B1-ButtonRelease> "group::lassoEnd $id %x %y; unbindOnRelease group$id"
#        } else {
#            onClick group $id group$id $x $y
#        }
#    }
#
#    proc lassoEnd {id x y} {
#        global lassoStart
#        if [info exists lassoStart] {
#            set x [.wiring.canvas canvasx $x]
#            set y [.wiring.canvas canvasy $y]
#            eval inGroupSelect $id $x $y $lassoStart
#            .wiring.canvas delete lasso
#            updateCanvas
#            unset lassoStart
#        }
#    }
#}

#trace add execution checkAddGroup enterstep tout
