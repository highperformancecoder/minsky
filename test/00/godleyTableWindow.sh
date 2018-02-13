#! /bin/sh

here=`pwd`
if test $? -ne 0; then exit 2; fi
tmp=/tmp/$$
mkdir $tmp
if test $? -ne 0; then exit 2; fi
cd $tmp
if test $? -ne 0; then exit 2; fi

fail()
{
    echo "FAILED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 1
}

pass()
{
    echo "PASSED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 0
}

trap "fail" 1 2 3 15
cat >input.tcl <<EOF
source $here/test/assert.tcl
proc afterMinskyStarted {} {
  minsky.load $here/examples/1Free.mky
  findObject GodleyIcon
  set id [minsky.openGodley]
  openGodley \$id
  \$id.displayStyle sign
  \$id.renderCanvasToSVG 1FreeBase.svg
  \$id.selectedCol 1
  \$id.selectedRow 2
  \$id.hoverRow 2
  \$id.hoverCol 2
  \$id.displayStyle DRCR
  \$id.renderCanvasToSVG 1Free11Selected.svg
  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

diff 1FreeBase.svg $here/test/renderedImages/1FreeBase.svg
if [ $? -ne 0 ]; then fail; fi

diff 1Free11Selected.svg $here/test/renderedImages/1Free11Selected.svg
if [ $? -ne 0 ]; then fail; fi

pass
