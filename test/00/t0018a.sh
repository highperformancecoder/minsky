#! /bin/sh

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

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source $here/GUI/library/init.tcl
use_namespace minsky
minsky.load $here/examples/GoodwinLinear.mky
openLogFile tmp1.dat
set nsteps 10
set out [open tmp2.dat w]
puts -nonewline \$out "#time"
foreach name [variables.values.#keys] {
  puts -nonewline \$out " \$name"
}
puts \$out ""

# prepare element accessors for later use
foreach name [variables.values.#keys] {
        variables.values.@elem \$name
}
use_namespace minsky

for {set step 0} {\$step<\$nsteps} {incr step} {
  step
  puts -nonewline \$out "[t]"
  foreach name [variables.values.#keys] {
    puts -nonewline \$out " [variables.values(\$name).value]"
  }
  puts \$out ""
}
close \$out
reset #closes tmp1.dat
tcl_exit
EOF

$here/GUI/minsky input.tcl
if test $? -ne 0; then fail; fi

diff tmp1.dat tmp2.dat
if test $? -ne 0; then fail; fi

pass
