source "assert.tcl"
minsky.load $argv(2)
set numItems [minsky.model.items.size]
for {set i 0} {$i<$numItems} {incr i} {
    minsky.model.items.@elem $i
    if [minsky.getItemAt [minsky.model.items($i).x] [minsky.model.items($i).y]] {
        set origClassType [minsky.canvas.item.classType]
        minsky.canvas.copyItem
        minsky.canvas.mouseUp -500 -500
        assert {[minsky.getItemAt -500 -500]}
        if {$origClassType=="IntOp"} {
            assert {"Variable:integral"==[minsky.canvas.item.classType]} "$argv(2)"
        } else {
            assert "\"$origClassType\"==\[minsky.canvas.item.classType\]" "$argv(2)"
        }
        minsky.canvas.item.moveTo -1000 -1000
    }
}
tcl_exit   
