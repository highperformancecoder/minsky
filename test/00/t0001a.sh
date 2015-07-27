export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

if [ x$MINSKY_TEST_DATABASE_PARAMS=x ]; then
    MINSKY_TEST_DATABASE_PARAMS=$HOME/.testDatabase
fi

here=`pwd`
mkdir /tmp/$$
cd /tmp/$$

cp -r $here/test/testEq.mky .
if [ -x $here/test/unittests ]; then
    $here/test/unittests
else
    exit 1;
fi
