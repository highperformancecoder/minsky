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
minsky.defaultFont Sans

proc afterMinskyStarted {} {
  minsky.load $here/examples/1Free.mky
  minsky.multipleEquities 1
  findObject GodleyIcon
  set id [minsky.openGodley]
  openGodley \$id
  \$id.displayStyle sign
  \$id.displayValues 1
  \$id.renderToSVG 1FreeBase.svg
  \$id.selectedCol 1
  \$id.selectedRow 2
  \$id.insertIdx 0
  \$id.selectIdx 3
  \$id.hoverRow 2
  \$id.hoverCol 2
  \$id.mouseMoveB1 [expr [lindex [\$id.colLeftMargin] 2]+10] [expr 4*[\$id.rowHeight]+[\$id.topTableOffset]]
  \$id.displayStyle DRCR
  \$id.renderToSVG 1Free11Selected.svg
  \$id.selectedCol 0
  \$id.renderToSVG 1FreeSelectedRow.svg
  \$id.selectedCol 1
  \$id.selectedRow 0
  \$id.mouseMoveB1 [expr [lindex [\$id.colLeftMargin] 2]+10] [expr 3*[\$id.rowHeight]+[\$id.topTableOffset]]
  \$id.renderToSVG 1FreeSelectedCol.svg
  

  minsky.load $here/examples/LoanableFunds.mky
  findObject GodleyIcon
  set id [minsky.openGodley]
  \$id.displayStyle DRCR
  \$id.displayValues 0
  \$id.renderToSVG LoanableFundsBase.svg

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
