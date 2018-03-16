#!../GUI/minsky
#use_namespace minsky
proc afterMinskyStarted {} {
 source "assert.tcl"
 minsky.load constantCopyProblem.mky
 step
 set bVal [variableValues.@elem :b]
 assert "\[$bVal.value 0 \]==12"
 tcl_exit
}
