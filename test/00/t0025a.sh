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
    pids=`ps -ef|grep firefox|grep minsky.html|cut -c10-19`
    if [ "$pids" != "" ]; then
        kill $pids
    fi
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

addPlot
doPlotOptions minsky.canvas.itemFocus
.pltWindowOptions.buttonBar.ok invoke

canvas.getItemAt [minsky.canvas.itemFocus.x] [minsky.canvas.itemFocus.y] 
plotDoubleClick [TCLItem]

tcl_exit
}
EOF

$here/gui-tk/minsky mainModals.tcl
if test $? -ne 0; then fail; fi

pass
