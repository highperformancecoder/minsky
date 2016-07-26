#!../GUI/minsky
#use_namespace minsky
proc afterMinskyStarted {} {
 source "assert.tcl"
 minsky.load constantCopyProblem.mky
 step
 set bVal [variableValues.@elem :b]
 assert \[$bVal.value\]==12
 resetEdited
 exit
}
