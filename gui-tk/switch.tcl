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

#proc newSwitch {id} {
#    global minskyHome
#    switchItem.get $id
#    .old_wiring.canvas create switch [switchItem.x] [switchItem.y] -id $id  -tags "switchItem$id switches"
#    .old_wiring.canvas lower switchItem$id
#
#    .old_wiring.canvas bind switchItem$id <Button-1> "onClick switchItem $id switchItem$id %x %y"
#    .old_wiring.canvas bind switchItem$id <<middleMouse-Motion>> \
#        "wires::extendConnect \[closestOutPort %x %y \] group$id %x %y"
#    .old_wiring.canvas bind switchItemgroup$id <<middleMouse-ButtonRelease>> \
#        "wires::finishConnect group$id %x %y"
#    .old_wiring.canvas bind group$id  <<contextMenu>> "rightMouseSwitch $id %x %y %X %Y"
#    .old_wiring.canvas bind switchItem$id  <Double-Button-1> "switchEdit $id"
#    .old_wiring.canvas bind switchItem$id <Enter> "itemEnterLeave switchItem $id switchItem$id 1"
#    .old_wiring.canvas bind switchItem$id <Leave> "itemEnterLeave switchItem $id switchItem$id 0"
#
#    
#}

proc placeNewSwitch {} {
    set id [minsky.newSwitch]
    switchItem.get $id
    switchItem.detailedText "Enter your note here"
    newItem $id
    global moveOffs$id.x moveOffs$id.y
    set moveOffs$id.x 0
    set moveOffs$id.y 0
    bind .old_wiring.canvas <Enter> "move $id %x %y"
    bind .old_wiring.canvas <Motion> "move $id %x %y"
    bind .old_wiring.canvas <Button-1> clearTempBindings
    bind . <Key-Escape> "clearTempBindings
       deleteSwitch $id
       .old_wiring.canvas delete switchItem$id"

}

proc rightMouseSwitch {id x y X Y} {
    group.get $id
    set var [group.selectVariable [.old_wiring.canvas canvasx $x] [.old_wiring.canvas canvasy $y]]
    if {$var==-1} {
        contextMenu group$id $X $Y
    } else {
        .old_wiring.context delete 0 end
        .old_wiring.context add command -label "Edit" -command "editItem $var var"
        var.get $var
        .old_wiring.context add command -label "Copy" -command "
           copyVar $var
           var.rotation 0
           var.set
        "
        .old_wiring.context add command -label "Remove" -command "
           group.removeVariableById $var
           group.set
        "
        .old_wiring.context post $X $Y

    }
}


proc incrCase {delta} {
    set item minsky.canvas.item
    $item.setNumCases [expr [$item.numCases]+$delta]
    canvas.requestRedraw
}

