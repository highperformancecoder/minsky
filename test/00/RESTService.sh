#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

$here/RESTService/minsky-RESTService -batch >output <<EOF
/minsky/variableValues/@elem/"constant:one"
EOF
if [ $? -ne 0 ]; then fail; fi

cat >reference <<EOF
/minsky/variableValues/@elem/"constant:one"=>{"csvDialog":{"spec":{"counter":false,"dataColOffset":0,"dataCols":[],"dataRowOffset":0,"decSeparator":".","dimensionCols":[],"dimensionNames":[],"dimensions":[],"dontFail":false,"duplicateKeyAction":"throwException","escape":"\u0000","headerRow":0,"horizontalDimName":"?","horizontalDimension":{"type":"string","units":""},"maxColumn":1000,"mergeDelimiters":false,"missingValue":NaN,"numCols":0,"quote":"\"","separator":","},"url":""},"detailedText":"","enableSlider":true,"godleyOverridden":false,"name":"constant:one","rhs":{},"sliderMax":-1.7976931348623157e+308,"sliderMin":1.7976931348623157e+308,"sliderStep":0,"sliderStepRel":false,"tensorInit":{},"tooltip":"","units":[],"unitsCached":false}
EOF

diff -q -w output reference
if [ $? -ne 0 ]; then fail; fi

pass
