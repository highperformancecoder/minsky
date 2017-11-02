# loads file specified on $argv(2), and saves it to $argv(3)
# used for checking that minsky does not modify the example files
if {$argc<3} {tcl_exit 1}

minsky.load $argv(2)
minsky.matlab $argv(2).m
tcl_exit
