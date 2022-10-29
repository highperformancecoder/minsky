source "assert.tcl"
minsky.load $argv(2)
minsky.doPushHistory 0
set numItems [minsky.model.items.size]

for {set i 0} {$i<$numItems} {incr i} {
    minsky.model.items.@elem $i
    if [minsky.getItemAt [minsky.model.items($i).x] [minsky.model.items($i).y]] {
        set origClassType [minsky.canvas.item.classType]
        # removing GodleyIcon removes dependent variables from model
        if {$origClassType=="Item" ||$origClassType=="GodleyIcon" } continue 
        minsky.canvas.copyItem
        minsky.canvas.mouseUp -500 -500
        assert {[minsky.getItemAt -500 -500]}
        if {$origClassType=="IntOp"} {
            assert {"Variable:integral"==[minsky.canvas.item.classType]} "$argv(2)"
        } else {
            assert "\"$origClassType\"==\[minsky.canvas.item.classType\]" "$argv(2)"
        }
        minsky.canvas.deleteItem
    }
}
tcl_exit   
