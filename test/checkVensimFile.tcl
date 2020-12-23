puts "checking $argv(2)"
eval minsky.importVensim $argv(2)
minsky.deleteAllUnits
minsky.step
tcl_exit
