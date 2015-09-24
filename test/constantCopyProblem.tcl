#!../GUI/minsky
#use_namespace minsky
proc afterMinskyStarted {} {
 source "assert.tcl"
 minsky.load constantCopyProblem.mky
 step
 set aVal [values.@elem :a]
 set bVal [values.@elem :b]
 assert \[$bVal.value\]==[$aVal.value]
 resetEdited
 exit
}
