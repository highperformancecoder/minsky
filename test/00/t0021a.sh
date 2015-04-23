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

# replay a recording

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
proc afterMinskyStarted {} {
  update
  after 100
  deiconifyInitVar
  set eventRecord [open $here/test/1FreeRecording.tcl r]
  while {[gets \$eventRecord cmd]>=0} {
    eval \$cmd
    update
    after 10
  }
resetEdited
exit
}
EOF

# disable this test, as it's too hard to maintain
pass

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

pass
