here=`pwd`
. $here/test/common-test.sh

xmllint --noout --schema http://www.w3.org/2001/XMLSchema.xsd $here/minsky.xsd
if test $? -ne 0; then fail; fi

for m in $here/examples/*.mky; do
  xmllint --noout --schema $here/minsky.xsd $m
  if test $? -ne 0; then fail; fi
  done

pass
