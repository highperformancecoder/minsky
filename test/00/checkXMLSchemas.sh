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

xmllint --noout --schema http://www.w3.org/2001/XMLSchema.xsd $here/minsky.xsd
if test $? -ne 0; then fail; fi

# for now, disable testing of the examples, until we're ready to move
# to the new schema
for m in $here/examples/*.mky; do
  xmllint --noout --schema $here/minsky.xsd $m
  if test $? -ne 0; then fail; fi
  done

pass
