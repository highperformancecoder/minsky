source "assert.tcl"
minsky.load $argv(2)
foreach item [minsky.items.#keys] {
    minsky.item.get $item
    set origClassType [minsky.item.classType]
    minsky.item.get [minsky.copyItem $item]
    assert "\"$origClassType\"==\[minsky.item.classType\]" "$argv(2)"
}
tcl_exit   
