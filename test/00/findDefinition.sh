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

# overrride tk_messageBox to capture presence in a variable
set msgBox 0
proc tk_messageBox args {
  global msgBox
  set msgBox 1
}
set item.x 0
set item.y 0
proc indicateCanvasItemInError {x y} {
  global item.x item.y
  set item.x \$x
  set item.y \$y
}

foreach item [minsky.items.#keys] {
  if {![catch var.get \$item]} {
#we have a variable
    set defId [findVariableDefinition \$item]
    findDefinition \$item
    if {\$defId==-1} {
      assert "\$msgBox" "tk_messageBox not posted"
    }
    switch [var.name] {
       case "undef" -
       case "param1" -
       case "3" {assert "\$defId==-1" "undefined should return -1"}
       case "foo" {
          var.get \$defId
          assert {[var.inputWired]}
          assert "\$item.x==[var.x]" "var coordx fail"
          assert "\$item.y==[var.y]" "var coordy fail"
       }
       case "int" {
          item.get \$defId
          assert {[item.classType]=="IntOp"}
          assert "\$item.x==[var.x]" "int coordx fail"
          assert "\$item.y==[var.y]" "int coordy fail"
       }
       case "bar" {
          item.get \$defId
          assert {[item.classType]=="GodleyIcon"}
          assert "\$item.x==[var.x]" "godley coordx fail"
          assert "\$item.y==[var.y]" "godley coordy fail"
       }
     }
  }
}
tcl_exit
EOF

$here/gui-tk/minsky input.tcl
if [ $? -ne 0 ]; then fail; fi

pass
