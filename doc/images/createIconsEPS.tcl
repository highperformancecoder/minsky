#!minsky
source $minskyHome/library/init.tcl
use_namespace minsky
GUI

# create eps icons for the buttons

foreach op [availableOperations] {
    if {$op=="numOps"} break
    # ignore some operations
    switch $op {
        "constant" -
        "copy" continue
    }
    clearAll
    addOperation $op
    renderCanvasToPS $op.eps
}
clearAll
newSwitch
renderCanvasToPS switchIcon.eps
clearAll
newVariable var flow
renderCanvasToPS var.eps
clearAll
newVariable const parameter
renderCanvasToPS const.eps
tcl_exit
