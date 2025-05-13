export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

if [ x$MINSKY_TEST_DATABASE_PARAMS=x ]; then
    MINSKY_TEST_DATABASE_PARAMS=$HOME/.testDatabase
fi

here=`pwd`
. test/common-test.sh

cp -r $here/test/testEq.mky $here/gui-js/apps/minsky-web/src/assets/images/icons/bank.svg $here/examples/1Free.mky .
if [ -x $here/test/unittests ]; then
    $here/test/unittests
    if [ $? -ne 0 ]; then fail; fi
else
    fail;
fi

if [ "$TRAVIS" = 1 ]; then pass; fi

for i in *.svg; do
    if [ $i="bank.svg" ]; then continue; fi
    $here/test/compareSVG.sh $i $here/test/renderedImages/$i
    if [ $? -ne 0 ]; then fail; fi
done

$here/test/compareSVG.sh 1FreePhillipsMutated.svg 1FreePhillipsMutatedLoaded.svg
if [ $? -ne 0 ]; then fail; fi
$here/test/compareSVG.sh 1FreePubNeither.svg 1FreeWiringEditorPubNot.svg
if [ $? -ne 0 ]; then fail; fi
$here/test/compareSVG.sh 1FreeWiringEditorPubEditor.svg 1FreeWiringNotPubEditor.svg
if [ $? -ne 0 ]; then fail; fi
pass
