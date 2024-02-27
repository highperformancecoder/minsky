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

# test disabled, as it is no longer relevant in Minsky 3.x
# TODO: to be refactored for 3.x, or removed entirely
pass

trap "fail" 1 2 3 15
cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.defaultFont Sans

proc afterMinskyStarted {} {
  minsky.load $here/examples/1Free.mky
  minsky.multipleEquities 1
  minsky.displayStyle sign
  minsky.displayValues 1
  findObject GodleyIcon
  set id [minsky.openGodley]
  openGodley \$id
  \$id.popup.enableButtons
  \$id.popup.renderToSVG 1FreeBase.svg
  \$id.popup.selectedCol 1
  \$id.popup.selectedRow 2
  \$id.popup.insertIdx 0
  \$id.popup.selectIdx 3
  \$id.popup.hoverRow 2
  \$id.popup.hoverCol 2
  \$id.popup.mouseMoveB1 [expr [lindex [\$id.popup.colLeftMargin] 2]+10] [expr 4*[\$id.popup.rowHeight]+[\$id.popup.topTableOffset]]
  minsky.displayStyle DRCR
  \$id.popup.renderToSVG 1Free11Selected.svg
  \$id.popup.selectedCol 0
  \$id.popup.renderToSVG 1FreeSelectedRow.svg
  \$id.popup.selectedCol 1
  \$id.popup.selectedRow 0
  \$id.popup.mouseMoveB1 [expr [lindex [\$id.popup.colLeftMargin] 2]+10] [expr 3*[\$id.popup.rowHeight]+[\$id.popup.topTableOffset]]
  \$id.popup.renderToSVG 1FreeSelectedCol.svg
  

  minsky.load $here/examples/LoanableFunds.mky
  findObject GodleyIcon
  set id [minsky.openGodley]
  minsky.displayStyle DRCR
  minsky.displayValues 0
  \$id.popup.enableButtons
  \$id.popup.renderToSVG LoanableFundsBase.svg

  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

for i in *.svg; do
    # ids are assigned randomly, so strip out those tags
    sed -e 's/id="[^"]*"/id=""/' <$i >tmp1
    sed -e 's/id="[^"]*"/id=""/' <$here/test/renderedImages/$i >tmp2
    $here/test/compareSVG.sh tmp1 tmp2
    if [ $? -ne 0 ]; then fail; fi
done

pass
