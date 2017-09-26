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
minsky.load $here/test/findDefinition.mky
set msgBox 0

for {set i 0} {\$i<[minsky.model.items.size]} {incr i} {
  minsky.model.items.@elem \$i
  set item minsky.model.items(\$i)
  if {[regexp "Variable:" [\$item.classType]]||[\$item.classType]=="VarConstant"} {
    minsky.canvas.getItemAt [\$item.x] [\$item.y]
    set searchItem [minsky.TCLItem]
    set findResult [minsky.canvas.findVariableDefinition]
    set foundItem [minsky.TCLItem]
    switch [\$item.name] {
       "undef" {assert "\$findResult==0" undef}
       "param1" -
       "3" {assert "\"\$searchItem\"==\"\$foundItem\"" paramConst}
       "foo" {
          assert "[\$foundItem.inputWired]"
          assert "[\$item.x]==[\$foundItem.x]" "var coordx fail"
          assert "[\$item.y]==[\$foundItem.y]" "var coordy fail"
       }
       "int" {
          assert "\"[\$foundItem.classType]\"==\"IntOp\"" "IntOp"
       }
       "bar" {
          assert "\"[\$foundItem.classType]\"==\"GodleyIcon\"" "GodleyIcon"
       }
     }
  }
}
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
