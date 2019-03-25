minsky.load /home/rks/github/minsky2/examples/1Free.mky
minsky.findObject GodleyIcon
minsky.canvas.item.table.exportToLaTeX 1FreeGodley.tex
minsky.canvas.item.table.exportToCSV 1FreeGodley.csv

minsky.load /home/rks/github/minsky2/test/testEq.mky
minsky.findObject GodleyIcon
minsky.canvas.item.table.exportToLaTeX testEqGodley.tex
minsky.canvas.item.table.exportToCSV testEqGodley.csv


tcl_exit
