#!../GUI/minsky
#use_namespace minsky
proc afterMinskyStarted {} {
 source "assert.tcl"
 minsky.load constantCopyProblem.mky
 step
 set aVal [variables.values.@elem :a]
 set bVal [variables.values.@elem :b]
 assert \[$bVal.value\]==[$aVal.value]
 resetEdited
 exit
}
