source "assert.tcl"
minsky.load $argv(2)
set numItems [minsky.model.items.size]
for {set i 0} {$i<$numItems} {incr i} {
    minsky.model.items.@elem $i
    set origClassType [minsky.model.items(0).classType]
    if [minsky.getItemAt [minsky.model.items(0).x] [minsky.model.items(0).y]] {
        minsky.canvas.copyItem
        minsky.canvas.mouseUp -500 -500
        assert {[minsky.getItemAt -500 -500]}
        assert "\"$origClassType\"==\[minsky.canvas.item.classType\]" "$argv(2)"
    }
}
tcl_exit   
