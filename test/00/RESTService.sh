#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

$here/RESTService/minsky-RESTService -batch >output <<EOF
/minsky/variableValues/@elem/"constant:one"
EOF
if [ $? -ne 0 ]; then fail; fi

cat >reference <<EOF
/minsky/variableValues/@elem/"constant:one"=>{"csvDialog":{"backgroundColour":{"a":1,"b":0.80000000000000004,"g":0.80000000000000004,"r":0.80000000000000004},"colWidth":50,"flashNameRow":false,"item":{},"resolutionScaleFactor":1,"spec":{"counter":false,"dataColOffset":0,"dataCols":[],"dataRowOffset":0,"decSeparator":".","dimensionCols":[],"dimensionNames":[],"dimensions":[],"dontFail":false,"duplicateKeyAction":"throwException","escape":"\\\\","headerRow":0,"horizontalDimName":"?","horizontalDimension":{"type":"string","units":""},"mergeDelimiters":false,"missingValue":NaN,"numCols":0,"quote":"\\"","separator":","},"url":"","wire":{},"xoffs":80},"detailedText":"","godleyOverridden":false,"name":"constant:one","rhs":{},"sliderVisible":false,"tensorInit":{},"tooltip":"","units":[],"unitsCached":false}
EOF

diff -q -w output reference
if [ $? -ne 0 ]; then fail; fi

pass
