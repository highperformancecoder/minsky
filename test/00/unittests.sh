export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

if [ x$MINSKY_TEST_DATABASE_PARAMS=x ]; then
    MINSKY_TEST_DATABASE_PARAMS=$HOME/.testDatabase
fi

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

cp -r $here/test/testEq.mky $here/gui-tk/icons/bank.svg $here/examples/1Free.mky .
if [ -x $here/test/unittests ]; then
    $here/test/unittests
    if [ $? -ne 0 ]; then fail; fi
else
    fail;
fi

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
