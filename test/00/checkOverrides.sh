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

cp $here/test/assert.tcl .
for i in $here/examples/*.mky; do
    echo $i
   $here/gui-tk/minsky $here/test/checkOverrides.tcl $i &>log
   if [ $? -ne 0 ]; then
       cat log
       fail
   fi
done

pass
