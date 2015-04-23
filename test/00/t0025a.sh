#! /bin/sh

here=`pwd`
if test $? -ne 0; then exit 2; fi
tmp=/tmp/$$
mkdir $tmp
if test $? -ne 0; then exit 2; fi
cd $tmp
if test $? -ne 0; then exit 2; fi

killFireFox()
{
    kill `ps -ef|grep firefox|grep minsky.html|cut -c10-19`
}

fail()
{
    echo "FAILED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    killFireFox
    exit 1
}

pass()
{
    echo "PASSED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    killFireFox
    exit 0
}

trap "fail" 1 2 3 15

cat >mainModals.tcl <<EOF
proc bgerror x {
  puts \$x
  exit 1
}

proc afterMinskyStarted {} {
showPreferences
.preferencesForm.buttonBar.ok invoke
deiconifyRKDataForm
topLevelHelp

set id [newPlot]
update
event generate .wiring.canvas <Button-1> 

doPlotOptions \$id
.pltWindowOptions.buttonBar.ok invoke

plotDoubleClick \$id
resizePlot \$id 100 100 2 2

deletePlot plot\$id \$id

resetEdited
exit
}
EOF

$here/GUI/minsky mainModals.tcl
if test $? -ne 0; then fail; fi

pass
