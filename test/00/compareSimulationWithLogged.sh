#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# check models reproduced logged data when run
cd $here/examples
for i in *.mky; do
    # math-examples is not a runnable example
    if [ $i = "math-examples.mky" ]; then continue; fi
    if [ $i = "indexing.mky" ]; then continue; fi
    if [ $i = "importedCSV.mky" ]; then continue; fi
    if [ $i = "histogram.mky" ]; then continue; fi
    python3 ../test/compareFileLog.py $i ../test/exampleLogs/$i.log
    if test $? -ne 0; then 
        echo "for $i"
        fail
    fi
done

pass
