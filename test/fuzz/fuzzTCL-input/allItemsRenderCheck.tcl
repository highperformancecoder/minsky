minsky.defaultFont Courier
minsky.setGodleyIconResource /home/rks/github/minsky2/gui-tk/icons/bank.svg
minsky.setGroupIconResource /home/rks/github/minsky2/gui-tk/icons/group.svg
minsky.load /home/rks/github/minsky2/test/allItems.mky
minsky.renderCanvasToSVG allItemsBare.svg

for {set i 0} {$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items.@elem $i
  minsky.model.items($i).mouseFocus 1
  minsky.model.items($i).tooltip tooltip
}
for {set i 0} {$i<[minsky.model.groups.size]} {incr i} {
  minsky.model.groups.@elem $i
  minsky.model.groups($i).mouseFocus 1
  minsky.model.groups($i).tooltip tooltip
}
minsky.renderCanvasToSVG allItemsMouseOver.svg

for {set i 0} {$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items($i).mouseFocus 0
  minsky.model.items($i).selected 1
}
for {set i 0} {$i<[minsky.model.groups.size]} {incr i} {
  minsky.model.groups($i).mouseFocus 0
  minsky.model.groups($i).selected 1
}
minsky.renderCanvasToSVG allItemsSelected.svg

tcl_exit
