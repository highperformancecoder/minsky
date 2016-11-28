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

cat >input.tcl <<EOF
source $here/test/assert.tcl
minsky.load $here/examples/GoodwinLinear02.mky
# find the group item
foreach item [minsky.items.#keys] {
  minsky.item.get \$item
  if {[minsky.item.classType]=="Group"} {
    minsky.newGroupTCL foo \$item
    minsky.group.get \$item
    assert {[foo.items.size]==[minsky.group.items.size]}
    foo.delete
    assert {[llength [info commands foo.*]]==0}
   }
}
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
