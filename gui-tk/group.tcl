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

proc rightMouseGroup {x y X Y} {
    if [selectVar $x $y] {
        .wiring.context delete 0 end
        .wiring.context add command -label "Edit" -command "editItem"
        .wiring.context add command -label "Copy" -command "canvas.copyItem"
        .wiring.context add command -label "Remove" -command "canvas.removeItemFromItsGroup"
        .wiring.context post $X $Y
    } else {
        contextMenu $x $y $X $Y
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

proc deiconifyEditGroup {} {
    if {![winfo exists .wiring.editGroup]} {
        toplevel .wiring.editGroup
        wm title .wiring.editGroup "Edit Group"
        wm transient .wiring.editGroup .wiring

        frame .wiring.editGroup.name
        label .wiring.editGroup.name.label -text "Name"
        entry .wiring.editGroup.name.val -width 20
        pack .wiring.editGroup.name.label .wiring.editGroup.name.val -side left

        frame .wiring.editGroup.rot
        label .wiring.editGroup.rot.label -text "     Rotation"
        entry .wiring.editGroup.rot.val -width 20
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

proc groupEdit {} {
    deiconifyEditGroup
    .wiring.editGroup.name.val delete 0 end
    .wiring.editGroup.name.val insert 0 [minsky.canvas.item.title]
    .wiring.editGroup.rot.val delete 0 end
    .wiring.editGroup.rot.val insert 0 [minsky.canvas.item.rotation]
    .wiring.editGroup.buttonBar.ok configure \
        -command {
            minsky.canvas.item.rotation [.wiring.editGroup.rot.val get]
            minsky.canvas.item.title [.wiring.editGroup.name.val get]
            minsky.canvas.requestRedraw
            closeEditWindow .wiring.editGroup
        }
    grab .wiring.editGroup
}

namespace eval group {

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
}

#trace add execution checkAddGroup enterstep tout
