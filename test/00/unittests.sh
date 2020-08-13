export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

if [ x$MINSKY_TEST_DATABASE_PARAMS=x ]; then
    MINSKY_TEST_DATABASE_PARAMS=$HOME/.testDatabase
fi

here=`pwd`
mkdir /tmp/$$
cd /tmp/$$

<<<<<<< HEAD:test/00/t0001a.sh
cp -r $here/test/testEq.mky .
cp $here/examples/GoodwinLinear02.mky .
=======
cp -r $here/test/testEq.mky $here/gui-tk/icons/bank.svg .
>>>>>>> master:test/00/unittests.sh
if [ -x $here/test/unittests ]; then
    $here/test/unittests
else
    exit 1;
fi
