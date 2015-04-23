#!minsky
source library/init.tcl
use_namespace minsky
GUI

# create gif icons for the Macintosh, see ticket #187

foreach op [availableOperations] {
    if {$op=="numOps"} break
    # ignore some operations
    switch $op {
        "constant" -
        "copy" -
        "integrate"  continue 
    }
    image create photo [set op]Img -width 24 -height 24
    operationIcon [set op]Img $op
    # I don't like the hardcoded background value here, but the
    # transparent icons appear ghastly on the Mac. The value #f0f0f0
    # is a resonable compormise
    [set op]Img write icons/$op.gif  -background #f0f0f0 -format gif
    #[set op]Img write icons/tmp.ppm -background #f0f0f0 -format ppm
    #catch {exec -- ppmquant 3 icons/tmp.ppm | ppmtogif -transparent #f0f0f0 >icons/$op.gif}
}
tcl_exit
