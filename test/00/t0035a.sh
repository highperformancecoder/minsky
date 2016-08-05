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

cp $here/examples/*.mky $here/test/testEq.mky $here/test/test1argBinaryDisplays.mky .
for i in *.mky; do
  echo $i
  $here/gui-tk/minsky $here/test/renderEquations.tcl $i
  diff $i.gif $here/test/renderedEquations/$i.gif
  # deal with different GIF renderers on developer machine as on Travis
  # test passes if either rendered equations match
  if test $? -ne 0; then 
      diff $i.gif $here/test/altRenderedEquations/$i.gif
      if test $? -ne 0; then fail; fi
  fi
done

pass
