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
    .wiring.canvas create item [wiringGroup.group.x] [wiringGroup.group.y] -id $id  -tags "item$id items"
    .wiring.canvas lower item$id

    .wiring.canvas bind item$id <Button-1> "group::button1 $id %x %y"
    .wiring.canvas bind item$id <<middleMouse-Motion>> \
        "wires::extendConnect \[closestOutPort %x %y \] group$id %x %y"
    .wiring.canvas bind item$id <<middleMouse-ButtonRelease>> \
        "wires::finishConnect item$id %x %y"
    .wiring.canvas bind item$id  <Double-Button-1> "groupEdit $id"
    .wiring.canvas bind item$id <Enter> "itemEnterLeave item $id item$id 1"
    .wiring.canvas bind item$id <Leave> "itemEnterLeave item $id item$id 0"

    
}

proc rightMouseGroup {id x y X Y} {
    set varId [selectVar $id [.wiring.canvas canvasx $x] [.wiring.canvas canvasy $y]]
    if {$varId>=0} {
        .wiring.context delete 0 end
        .wiring.context add command -label "Edit" -command "item.get $varId; editVar"
        .wiring.context add command -label "Copy" -command "
           item.get $varId
           copyVar 
           var.rotation 0
           var.set
        "
        .wiring.context add command -label "Remove" -command "
           wiringGroup.group.removeVariableById 
           wiringGroup.group.set
        "
        .wiring.context post $X $Y
    } else {
        contextMenu $id $X $Y
    }
}


proc deleteGroupItem {id} {
    deleteGroup $id
    .wiring.canvas delete all
    clearAllGetterSetters
    .wiring.canvas delete all
    updateCanvas
}

proc ungroupGroupItem {id} {
    ungroup $id
    .wiring.canvas delete all
    clearAllGetterSetters
    .wiring.canvas delete all
    updateCanvas
}

proc lasso {x y} {
    global lassoStart
    set x [.wiring.canvas canvasx $x]
    set y [.wiring.canvas canvasy $y]
    if {![info exists lassoStart]} {
        set lassoStart "$x $y"
        .wiring.canvas create rectangle $x $y $x $y -tag lasso
    }
    eval .wiring.canvas coords lasso $lassoStart $x $y
}

proc lassoEnd {x y} {
    global lassoStart
    if [info exists lassoStart] {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        eval select $x $y $lassoStart
        foreach item [eval .wiring.canvas find all] {
            .wiring.canvas coords $item [.wiring.canvas coords $item]
        }
        .wiring.canvas delete lasso
        unset lassoStart
        updateCanvas
    }
}

proc groupContext {id x y} {
    .wiring.context delete 0 end
    .wiring.context add command -label Help -command {help Group}
    .wiring.context add command -label Description -command "postNote group $id"
    .wiring.context add command -label "Edit" -command "groupEdit $id"
    .wiring.context add command -label "Open in canvas" -command "openInCanvas $id"
    .wiring.context add command -label "Zoom to display" -command "group::zoomToDisplay $id"
    .wiring.context add command -label "Remove plot icon" -command "group.get $id; group.removeDisplayPlot"
    .wiring.context add command -label "Resize" -command "group::resize $id"
    .wiring.context add command -label "Copy" -command "group::copy $id"
    .wiring.context add command -label "Save group as" -command "group::save $id"
    .wiring.context add command -label "Flip" -command "group::flip $id"
    .wiring.context add command -label "Flip Contents" -command "group::flipContents $id"
    .wiring.context add command -label "Browse object" -command "group.get $id; obj_browser wiringGroup.group.*"
    .wiring.context add command -label "Group" -command "wiringGroup.createGroup;.wiring.canvas delete all; updateCanvas"
    .wiring.context add command -label "Ungroup" -command "ungroupGroupItem $id"
    .wiring.context add command -label "Raise" -command "raiseItem group$id"
    .wiring.context add command -label "Lower" -command "lowerItem group$id"
    .wiring.context add command -label "Delete" -command "deleteGroupItem $id"
    .wiring.context add command -label "content bounds" -command "
      wiringGroup.group.get $id
      .wiring.canvas create rectangle \[wiringGroup.group.cBounds\]
     "
}

proc deiconifyEditGroup {} {
    if {![winfo exists .wiring.editGroup]} {
        toplevel .wiring.editGroup
        wm title .wiring.editGroup "Edit Group"
        wm transient .wiring.editGroup .wiring

        frame .wiring.editGroup.name
        label .wiring.editGroup.name.label -text "Name"
        entry  .wiring.editGroup.name.val -width 20
        pack .wiring.editGroup.name.label .wiring.editGroup.name.val -side left

        frame .wiring.editGroup.rot
        label .wiring.editGroup.rot.label -text "     Rotation"
        entry  .wiring.editGroup.rot.val -width 20
        pack .wiring.editGroup.rot.label .wiring.editGroup.rot.val -side left

        pack .wiring.editGroup.name .wiring.editGroup.rot
        
        frame .wiring.editGroup.buttonBar
        button .wiring.editGroup.buttonBar.ok -text OK
        button .wiring.editGroup.buttonBar.cancel -text Cancel -command {
            closeEditWindow .wiring.editGroup}
        pack .wiring.editGroup.buttonBar.ok .wiring.editGroup.buttonBar.cancel -side left
        pack .wiring.editGroup.buttonBar -side bottom
        
        bind .wiring.editGroup <Key-Return> {invokeOKorCancel .wiring.editGroup.buttonBar}
        bind .wiring.editGroup <Key-Escape> {.wiring.editGroup.buttonBar.cancel invoke}
    } else {
        wm deiconify .wiring.editGroup
    }
    update
}

proc groupEdit {id} {
    wiringGroup.group.get $id
    deiconifyEditGroup
    .wiring.editGroup.name.val delete 0 end
    .wiring.editGroup.name.val insert 0 [group.title]
    .wiring.editGroup.rot.val delete 0 end
    .wiring.editGroup.rot.val insert 0 [group.rotation]
    .wiring.editGroup.buttonBar.ok configure \
        -command {
            wiringGroup.group.rotation [.wiring.editGroup.rot.val get]
            wiringGroup.group.title [.wiring.editGroup.name.val get]
            closeEditWindow .wiring.editGroup
        }
    grab .wiring.editGroup
}

namespace eval group {

    proc resize {id} {
        wiringGroup.group.get $id
        set bbox [.wiring.canvas bbox item$id]
        set item [eval .wiring.canvas create rectangle $bbox -tags resizeBBox]
        # disable lasso mode
        bind .wiring.canvas <Button-1> ""
        bind .wiring.canvas <B1-Motion> ""
        bind .wiring.canvas <B1-ButtonRelease> ""
        bind .wiring.canvas <Motion> "group::resizeRect $item %x %y"
        bind .wiring.canvas <ButtonRelease> "group::resizeItem $item $id %x %y"
    }

    # resize the bounding box to indicate how big we want the icon to be
    proc resizeRect {item x y} {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        set w [expr abs($x-[group.x])]
               set h [expr abs($y-[group.y])]
        .wiring.canvas coords $item  [expr [group.x]-$w] [expr [group.y]-$h] \
            [expr [group.x]+$w] [expr [group.y]+$h]
    }

    # compute width and height and redraw item
    proc resizeItem {item id x y} {
        set x [.wiring.canvas canvasx $x]
        set y [.wiring.canvas canvasy $y]
        .wiring.canvas delete $item
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
        .wiring.canvas delete group$id
        newGroupItem $id
        foreach p [wiringGroup.group.ports]  {
            adjustWire $p
        }
        bind .wiring.canvas <Motion> {}
        bind .wiring.canvas <ButtonRelease> {}
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
        .wiring.canvas delete group$id
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
        .wiring.canvas delete all
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
            .wiring.canvas bind group$id <B1-Motion> "lasso %x %y"
           .wiring.canvas bind group$id <B1-ButtonRelease> "group::lassoEnd $id %x %y; unbindOnRelease group$id"
        } else {
            onClick $id item$id $x $y
        }
    }

    proc lassoEnd {id x y} {
        global lassoStart
        if [info exists lassoStart] {
            set x [.wiring.canvas canvasx $x]
            set y [.wiring.canvas canvasy $y]
            eval inGroupSelect $id $x $y $lassoStart
#            .wiring.canvas delete lasso
            .wiring.canvas delete all
            updateCanvas
            unset lassoStart
        }
    }
}

#trace add execution checkAddGroup enterstep tout
