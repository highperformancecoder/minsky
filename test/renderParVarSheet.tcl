#!minsky

# a crummy little utility for dumping the rendered equations as a gif file
minsky.defaultFont Sans

proc afterMinskyStarted {} {
    uplevel #0 {
        minsky.load $argv(2)
        minsky.variableSheet.renderToSVG "$argv(2)-variables.svg"
        minsky.parameterSheet.renderToSVG "$argv(2)-parameters.svg"
        tcl_exit
    }
}
