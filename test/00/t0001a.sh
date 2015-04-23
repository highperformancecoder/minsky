export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
cd test
if [ -x unittests ]; then
    unittests
else
    exit 1;
fi
