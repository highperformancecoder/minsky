export LD_LIBRARY_PATH=$HOME/usr/lib:/usr/local/lib:$LD_LIBRARY_PATH
EXIT=0

here=`pwd`
mkdir /tmp/$$
cd /tmp/$$

for i in $here/examples/*.mky; do
    if [ $i = "$here/examples/EndogenousMoney.mky" ]; then continue; fi
    "$here/GUI/minsky" "$here/test/rewriteMky.tcl" "$i"  tmp;
    if test $? -ne 0; then fail; fi
    $here/test/cmpFp "$i" tmp
    if [ $? -ne 0 ]; then
        echo "$i mutates"
        EXIT=1
    fi
done

rm -rf /tmp/$$
exit $EXIT
