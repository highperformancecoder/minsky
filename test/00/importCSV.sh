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
  minsky.addVariable par parameter
  assert {[findObject Variable:parameter]}  
  editItem
  assert {[winfo ismapped .wiring.editVar]}
  .wiring.editVar.buttonBar.import invoke
  assert {[winfo ismapped .wiring.csvImport]}      
  #.wiring.csvImport.fileUrl.load invoke    
  minsky.value.csvDialog.url $here/test/testEqGodley.csv
  minsky.value.csvDialog.loadFile  
  minsky.value.csvDialog.requestRedraw        
  minsky.value.csvDialog.spec.guessFromFile minsky.value.csvDialog.url      
  # Both this button invoke and csvImportDialogOK fail with segmentation fault here and I don't understand why...
  #after 2000 {
  #    focus .wiring.csvImport
      .wiring.csvImport.buttonBar.ok invoke
  #}
  #csvImportDialogOK
 
  after 3000 {tcl_exit}
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
