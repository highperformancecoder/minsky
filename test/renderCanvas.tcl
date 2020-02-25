#!minsky

# a crummy little utility for dumping the rendered canvases as a svg files
minsky.defaultFont Sans
minsky.setGodleyIconResource $minskyHome/icons/bank.svg
minsky.setGroupIconResource $minskyHome/icons/group.svg
minsky.srand 10
minsky.load $argv(2)
minsky.renderCanvasToSVG $argv(3)
tcl_exit

