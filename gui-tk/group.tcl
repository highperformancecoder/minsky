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

# Group (or block) functionality

# convert degrees to radian
proc radian {deg} {
    return [expr $deg*3.1415927/180]
}

setGroupIconResource $minskyHome/icons/group.svg

proc newGroupItem {id} {
    global minskyHome
    wiringGroup.group.get $id
    .old_wiring.canvas create item [wiringGroup.group.x] [wiringGroup.group.y] -id $id  -tags "item$id items"
    .old_wiring.canvas lower item$id

    .old_wiring.canvas bind item$id <Button-1> "group::button1 $id %x %y"
    .old_wiring.canvas bind item$id <<middleMouse-Motion>> \
        "wires::extendConnect \[closestOutPort %x %y \] group$id %x %y"
    .old_wiring.canvas bind item$id <<middleMouse-ButtonRelease>> \
        "wires::finishConnect item$id %x %y"
    .old_wiring.canvas bind item$id  <Double-Button-1> "groupEdit $id"
    .old_wiring.canvas bind item$id <Enter> "itemEnterLeave item $id item$id 1"
    .old_wiring.canvas bind item$id <Leave> "itemEnterLeave item $id item$id 0"

    
}

proc rightMouseGroup {id x y X Y} {
    set varId [selectVar $id [.old_wiring.canvas canvasx $x] [.old_wiring.canvas canvasy $y]]
    if {$varId>=0} {
        .old_wiring.context delete 0 end
        .old_wiring.context add command -label "Edit" -command "item.get $varId; editVar"
        .old_wiring.context add command -label "Copy" -command "
           item.get $varId
           copyVar 
           var.rotation 0
           var.set
        "
        .old_wiring.context add command -label "Remove" -command "
           wiringGroup.group.removeVariableById 
           wiringGroup.group.set
        "
        .old_wiring.context post $X $Y
    } else {
        contextMenu $id $X $Y
    }
}


proc deleteGroupItem {id} {
    deleteGroup $id
    .old_wiring.canvas delete all
    clearAllGetterSetters
    .old_wiring.canvas delete all
    updateCanvas
}

proc ungroupGroupItem {id} {
    ungroup $id
    .old_wiring.canvas delete all
    clearAllGetterSetters
    .old_wiring.canvas delete all
    updateCanvas
}

proc lasso {x y} {
    global lassoStart
    set x [.old_wiring.canvas canvasx $x]
    set y [.old_wiring.canvas canvasy $y]
    if {![info exists lassoStart]} {
        set lassoStart "$x $y"
        .old_wiring.canvas create rectangle $x $y $x $y -tag lasso
    }
    eval .old_wiring.canvas coords lasso $lassoStart $x $y
}

proc lassoEnd {x y} {
    global lassoStart
    if [info exists lassoStart] {
        set x [.old_wiring.canvas canvasx $x]
        set y [.old_wiring.canvas canvasy $y]
        eval select $x $y $lassoStart
        foreach item [eval .old_wiring.canvas find all] {
            .old_wiring.canvas coords $item [.old_wiring.canvas coords $item]
        }
        .old_wiring.canvas delete lasso
        unset lassoStart
        updateCanvas
    }
}

proc groupContext {id x y} {
    .old_wiring.context delete 0 end
    .old_wiring.context add command -label Help -command {help Group}
    .old_wiring.context add command -label Description -command "postNote group $id"
    .old_wiring.context add command -label "Edit" -command "groupEdit $id"
    .old_wiring.context add command -label "Open in canvas" -command "openInCanvas $id"
    .old_wiring.context add command -label "Zoom to display" -command "group::zoomToDisplay $id"
    .old_wiring.context add command -label "Remove plot icon" -command "group.get $id; group.removeDisplayPlot"
    .old_wiring.context add command -label "Resize" -command "group::resize $id"
    .old_wiring.context add command -label "Copy" -command "group::copy $id"
    .old_wiring.context add command -label "Save group as" -command "group::save $id"
    .old_wiring.context add command -label "Flip" -command "group::flip $id"
    .old_wiring.context add command -label "Flip Contents" -command "group::flipContents $id"
    .old_wiring.context add command -label "Browse object" -command "group.get $id; obj_browser wiringGroup.group.*"
    .old_wiring.context add command -label "Ungroup" -command "ungroupGroupItem $id"
    .old_wiring.context add command -label "Raise" -command "raiseItem group$id"
    .old_wiring.context add command -label "Lower" -command "lowerItem group$id"
    .old_wiring.context add command -label "Delete" -command "deleteGroupItem $id"
    .old_wiring.context add command -label "content bounds" -command "
      wiringGroup.group.get $id
      .old_wiring.canvas create rectangle \[wiringGroup.group.cBounds\]
     "
}

proc deiconifyEditGroup {} {
    if {![winfo exists .old_wiring.editGroup]} {
        toplevel .old_wiring.editGroup
        wm title .old_wiring.editGroup "Edit Group"
        wm transient .old_wiring.editGroup .old_wiring

        frame .old_wiring.editGroup.name
        label .old_wiring.editGroup.name.label -text "Name"
        entry  .old_wiring.editGroup.name.val -width 20
        pack .old_wiring.editGroup.name.label .old_wiring.editGroup.name.val -side left

        frame .old_wiring.editGroup.rot
        label .old_wiring.editGroup.rot.label -text "     Rotation"
        entry  .old_wiring.editGroup.rot.val -width 20
        pack .old_wiring.editGroup.rot.label .old_wiring.editGroup.rot.val -side left

        pack .old_wiring.editGroup.name .old_wiring.editGroup.rot
        
        frame .old_wiring.editGroup.buttonBar
        button .old_wiring.editGroup.buttonBar.ok -text OK
        button .old_wiring.editGroup.buttonBar.cancel -text Cancel -command {
            closeEditWindow .old_wiring.editGroup}
        pack .old_wiring.editGroup.buttonBar.ok .old_wiring.editGroup.buttonBar.cancel -side left
        pack .old_wiring.editGroup.buttonBar -side bottom
        
        bind .old_wiring.editGroup <Key-Return> {invokeOKorCancel .old_wiring.editGroup.buttonBar}
        bind .old_wiring.editGroup <Key-Escape> {.old_wiring.editGroup.buttonBar.cancel invoke}
    } else {
        wm deiconify .old_wiring.editGroup
    }
    update
}

proc groupEdit {id} {
    wiringGroup.group.get $id
    deiconifyEditGroup
    .old_wiring.editGroup.name.val delete 0 end
    .old_wiring.editGroup.name.val insert 0 [group.title]
    .old_wiring.editGroup.rot.val delete 0 end
    .old_wiring.editGroup.rot.val insert 0 [group.rotation]
    .old_wiring.editGroup.buttonBar.ok configure \
        -command {
            wiringGroup.group.rotation [.old_wiring.editGroup.rot.val get]
            wiringGroup.group.title [.old_wiring.editGroup.name.val get]
            closeEditWindow .old_wiring.editGroup
        }
    grab .old_wiring.editGroup
}

namespace eval group {

    proc resize {id} {
        wiringGroup.group.get $id
        set bbox [.old_wiring.canvas bbox item$id]
        set item [eval .old_wiring.canvas create rectangle $bbox -tags resizeBBox]
        # disable lasso mode
        bind .old_wiring.canvas <Button-1> ""
        bind .old_wiring.canvas <B1-Motion> ""
        bind .old_wiring.canvas <B1-ButtonRelease> ""
        bind .old_wiring.canvas <Motion> "group::resizeRect $item %x %y"
        bind .old_wiring.canvas <ButtonRelease> "group::resizeItem $item $id %x %y"
    }

    # resize the bounding box to indicate how big we want the icon to be
    proc resizeRect {item x y} {
        set x [.old_wiring.canvas canvasx $x]
        set y [.old_wiring.canvas canvasy $y]
        set w [expr abs($x-[group.x])]
               set h [expr abs($y-[group.y])]
        .old_wiring.canvas coords $item  [expr [group.x]-$w] [expr [group.y]-$h] \
            [expr [group.x]+$w] [expr [group.y]+$h]
    }

    # compute width and height and redraw item
    proc resizeItem {item id x y} {
        set x [.old_wiring.canvas canvasx $x]
        set y [.old_wiring.canvas canvasy $y]
        .old_wiring.canvas delete $item
        set scalex [expr 2*abs($x-[group.x])/double([group.width])]
        set scaley [expr 2*abs($y-[group.y])/double([group.height])]
        # compute rotated scale factors
        set angle [radian [wiringGroup.group.rotation]]
        set rx [expr $scalex*cos($angle)-$scaley*sin($angle)]
        set ry [expr $scalex*sin($angle)+$scaley*cos($angle)]
        wiringGroup.group.width [expr abs($rx*[wiringGroup.group.width])]
        wiringGroup.group.height [expr abs($ry*[wiringGroup.group.height])]
        wiringGroup.group.computeDisplayZoom
        wiringGroup.group.set
        .old_wiring.canvas delete group$id
        newGroupItem $id
        foreach p [wiringGroup.group.ports]  {
            adjustWire $p
        }
        bind .old_wiring.canvas <Motion> {}
        bind .old_wiring.canvas <ButtonRelease> {}
    }

    proc copy {id} {
        wiringGroup.item.get $id
        insertNewGroup [copyItem $id]
    }

    proc save {id} {
        global workDir
        set fname [tk_getSaveFile -defaultextension .mky -initialdir $workDir]
        if [string length $fname] {
            saveGroupAsFile $id $fname
        }
    }

    proc flip {id} {
        group.get $id
        group.rotation [expr [group.rotation]+180]
        group.updatePortLocation
        group.set
        .old_wiring.canvas delete group$id
        newGroupItem $id
        foreach p [group.ports] {
            adjustWire $p
        }
    }

    proc flipContents {id} {
        group.get $id
        group.flipContents
        group.set
        # a bit kludgy to do a full redraw here...
        .old_wiring.canvas delete all
        updateCanvas
    }

    proc zoomToDisplay {id} {
        group.get $id
        set factor [expr 1.1*[group.computeDisplayZoom]/[group.zoomFactor]]
        zoomAt [group.x] [group.y] $factor
    }

    proc button1 {id x y} {
        group.get $id
        if [group.displayContents] {
            # use lasso mode when zoomed in
            lasso $x $y
            .old_wiring.canvas bind group$id <B1-Motion> "lasso %x %y"
           .old_wiring.canvas bind group$id <B1-ButtonRelease> "group::lassoEnd $id %x %y; unbindOnRelease group$id"
        } else {
            onClick $id item$id $x $y
        }
    }

    proc lassoEnd {id x y} {
        global lassoStart
        if [info exists lassoStart] {
            set x [.old_wiring.canvas canvasx $x]
            set y [.old_wiring.canvas canvasy $y]
            eval inGroupSelect $id $x $y $lassoStart
#            .old_wiring.canvas delete lasso
            .old_wiring.canvas delete all
            updateCanvas
            unset lassoStart
        }
    }
}

#trace add execution checkAddGroup enterstep tout
