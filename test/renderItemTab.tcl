#!minsky

minsky.defaultFont Sans

proc afterMinskyStarted {} {
    uplevel #0 {
        minsky.load $argv(2)
        minsky.variableTab.renderToSVG "$argv(2)-variables.svg"
        minsky.parameterTab.renderToSVG "$argv(2)-parameters.svg"
        tcl_exit
    }
}
