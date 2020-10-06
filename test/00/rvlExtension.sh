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
source $here/assert.tcl
proc afterMinskyStarted {} {
  minsky.load $here/examples/1Free.mky
  assert {[minsky.model.defaultExtension]==".mky"}
  # .mky files should be first
  assert {[lindex [fileTypes [minsky.model.defaultExtension]] 0]=={"Minsky" .mky TEXT}}

  minsky.addRavel
  # now .rvl files should be first
  assert {[minsky.model.defaultExtension]==".rvl"}
  assert {[lindex [fileTypes [minsky.model.defaultExtension]] 0]=={"Ravel" .rvl TEXT}}
  tcl_exit
}
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
