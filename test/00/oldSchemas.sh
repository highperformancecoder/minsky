here=`pwd`
. $here/test/common-test.sh

# ensure pyminsky module can be found
export PYTHONPATH=$here
# check that old schema can still be read correctly
cd $here/test/oldSchema
for i in */*.mky; do
    # this example has a constant and variable of the same name, so will fail this test
    echo $i
    if [ $i = schema0/4MonetaryMinskyModelLessUnstableStart.mky ]; then continue; fi
    python3 $here/test/rewriteMky.py $i $tmp/tmp.mky
    if test $? -ne 0; then fail; fi
    python3 $here/test/rewriteMky.py $tmp/tmp.mky $tmp/tmp1.mky
    if test $? -ne 0; then fail; fi
    python3 $here/test/rewriteMky.py $tmp/tmp1.mky $tmp/tmp2.mky
    if test $? -ne 0; then fail; fi

    $here/test/cmpFp  $tmp/tmp1.mky $tmp/tmp2.mky
    if test $? -ne 0; then
        echo "old schema file $i failed to convert"
        fail
    fi
    # check that the converted file is not an empty model
    if [ `wc -l $tmp/tmp2.mky|cut -f1 -d' '` -lt 50 ]; then
        echo "old schema file $i failed to convert"
        fail
    fi        
done 

# skip reference image checking on Travis CI
if [ -n "$TRAVIS" ]; then
    pass
fi

# prepare directory structure
for i in *; do
    mkdir -p $tmp/$i
done

pwd 
for i in */*.mky; do
    # mutating always
    if [ "$i" = schema1/EndogenousMoney.mky ]; then continue; fi
    echo "Rendering $i"
    python3 $here/test/renderCanvas.py $i $tmp/$i.svg
    $here/test/compareSVG.sh $tmp/$i.svg $i.svg
    if test $? -ne 0; then
        echo "rendered $i canvas mutated"
        fail
    fi
done

pass
