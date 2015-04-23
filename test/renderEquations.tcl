#!minsky

# a crummy little utility for dumping the rendered equations as a gif file

proc afterMinskyStarted {} {
    uplevel #0 {
        minsky.load $argv(2)
        image create photo equations -width 1000 -height 1000
        renderEquationsToImage equations

        equations write $argv(2).gif -format gif -background white
        resetEdited
        exit
    }
}
