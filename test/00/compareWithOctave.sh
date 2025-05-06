#! /bin/sh

here=`pwd`
. $here/test/common-test.sh

# checks whether Runge-Kutta solver works correctly on example files
# by comparing with Octave
cp -r $here/examples .
for i in examples/*.mky; do
    # math-examples is not a runnable example
    if [ $i = "examples/math-examples.mky" ]; then continue; fi
    # data and tensor objects not suppported in Octave
    if [ $i = "examples/binaryInterpolation.mky" ]; then continue; fi
    if [ $i = "examples/data-example.mky" ]; then continue; fi
    if [ $i = "examples/importedCSV.mky" ]; then continue; fi
    if [ $i = "examples/rand-gather-interpolation.mky" ]; then continue; fi
    if [ $i = "examples/tensor-switch.mky" ]; then continue; fi
    if [ $i = "examples/inner-outer-example.mky" ]; then continue; fi
    if [ $i = "examples/histogram.mky" ]; then continue; fi
    if [ $i = "examples/differenceOps.mky" ]; then continue; fi
    echo $i
    
    for order in 1 2 4; do
        for implicit in 0 1; do 
            #not available
            if [ $order -eq 1 -a $implicit -eq 0 ]; then continue; fi
            # This example needs higher order solvers
            if [ $i = "examples/4MonetaryMinskyModelLessUnstableStart.mky" -a \
                    $order -eq 1 ]; then continue; fi
            # derivative for tensor operations not yet implemented, so implicit methods not supported
            if [ $i = "examples/reductionExample.mky" -a $implicit ]; then continue; fi
            if [ $i = "examples/reductionsTest.mky" -a $implicit ]; then continue; fi
            if [ $i = "examples/scansTest.mky" -a $implicit ]; then continue; fi
            if [ $i = "examples/indexing.mky" -a $implicit  ]; then continue; fi
            if [ $i = "examples/statistics.mky" -a $implicit  ]; then continue; fi
            if [ $i = "examples/plotWidget.mky" ]; then continue; fi
            python3 $here/test/compareWithOctave.py -o $order -i $implicit $i
            if test $? -ne 0; then echo $i; fail; fi
            octave --no-window-system --silent $here/test/compareWithOctave.m $i
            if test $? -ne 0; then echo "$i failed"; fail; fi
        done
    done
done
pass
