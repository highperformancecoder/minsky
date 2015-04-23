#!/opt/local/bin/tclsh

set minskyHome ../GUI

source ../GUI/godley.tcl

proc a {s} {
    set prefix ""
    set varName ""
    return [list $s [list [parse_input $s prefix varName] $prefix $varName]]
}

set testset {
"variable"
" variable"
" variable "
"variable "
"-variable"
"--variable"
"- variable"
" - variable"
" -- variable"
"cr variable"
" cr variable"
" cr  variable"
" cr -variable"
"Dr variable"
" dR variable"
"  dr  variable"
" DR -variable"
" DR -variable "
" DR --variable"
" DR - variable"
" DR --- variable"
" DR -variable = 98.0"
"DR = 98.0"
"- = 98.0"
"dr - a = 98.0"
""
" "
"-"
"-dr a"
"--"
"dr"
"dr dog"
"- dog"
"-dr"
"dr - dr - dr e"
" - dr e"
"dog"
":foo"
"[1]:foo"
}

set resultset  {
variable {1 {} variable}
{ variable} {1 {} variable}
{ variable } {1 {} variable}
{variable } {1 {} variable}
-variable {1 - variable}
--variable {1 - variable}
{- variable} {1 - variable}
{ - variable} {1 - variable}
{ -- variable} {1 - variable}
{cr variable} {1 CR variable}
{ cr variable} {1 CR variable}
{ cr  variable} {1 CR variable}
{ cr -variable} {0 {} {}}
{Dr variable} {1 DR variable}
{ dR variable} {1 DR variable}
{  dr  variable} {1 DR variable}
{ DR -variable} {0 {} {}}
{ DR -variable } {0 {} {}}
{ DR --variable} {0 {} {}}
{ DR - variable} {0 {} {}}
{ DR --- variable} {0 {} {}}
{ DR -variable = 98.0} {0 {} {}}
{DR = 98.0} {0 {} {}}
{- = 98.0} {0 {} {}}
{dr - a = 98.0} {0 {} {}}
{} {1 {} {}}
{ } {1 {} {}}
- {0 {} {}}
{-dr a} {1 - {dr a}}
-- {0 {} {}}
dr {1 {} dr}
{dr dog} {1 DR dog}
{- dog} {1 - dog}
-dr {1 - dr}
{dr - dr - dr e} {0 {} {}}
{ - dr e} {1 - {dr e}}
dog {1 {} dog}
":foo" {1 {} :foo}
"[1]:foo" {1 {} {[1]:foo}}
}

proc genTestResults {} {
    foreach {l} $::testset {
	set res [a $l]
	puts stdout $res
    }
}

proc runtest {} {
    puts -nonewline stdout "testing resultset......"
    flush stdout
    set failed 0
    foreach {l r} $::resultset {
        set res [lindex [a $l] 1]
	if {[lindex $res 0]==0 && [lindex $r 0]==0 || $res==$r} {
	    # puts stdout "OK \"$l\""
	} else {
	puts stdout "FAIL \"$l\" = $res expected $r"
	set failed 1
	}
    }
    if {!$failed} {puts stdout passed} else {tcl_exit $failed}
}

proc runtest2 {} {
    set failed 0
    puts -nonewline stdout "testset chaining......."
    flush stdout
    foreach {l} $::testset {
	incr failed [chainingCheck $l]
    }
    if {!$failed} {puts stdout passed} else {tcl_exit $failed}
}

proc chainingCheck {l} {
    set failed 0
    set res [lindex [a $l] 1]
    foreach {ret prefix var} $res {}
    set res2 [lindex [a [concat $prefix $var]] 1]
    foreach {ret2 prefix2 var2} $res2 {}
    if {($ret != $ret2) && ($prefix != "") && ($var != "")} { incr failed 1; puts stdout "FAIL chaining for \"$l\" \"[set $v1]\" != \"[set $v2]\""}
    foreach {v1 v2} {prefix prefix2 var var2} {
	if {[set $v1] != [set $v2]} { incr failed 1; puts stdout "FAIL chaining for \"$l\" \"[set $v1]\" != \"[set $v2]\""}
    }
    return $failed
}

proc runtest3 {} {
    set failed 0
    puts -nonewline stdout "random test chaining...."
    flush stdout
    set tokens {dr CR variable drvar - -- " " = "!@#~$%^&*()_+{}[]:'\"\\,<.>?/"}
    set i 0
    set j 0
    while {1} {
	incr j
	set l {}
	foreach {ii} {1 2 3 4 5 6 7 8 9 10} {
	   set l [concat $l " " [lindex $tokens [expr int(rand()*9)]]]
	}
	incr failed [chainingCheck $l]
	if  {![expr $j%100]} {
	    incr i
	    puts -nonewline stdout [lindex {"\b|" "\b\\" "\b-" "\b/"} [expr $i%4]]
	    flush stdout
	    }
        if {$j>100000} {puts -nonewline stdout "\b"; break}
    }
    if {!$failed} {puts stdout passed} else {tcl_exit $failed}
}

#genTestResults
runtest
runtest2
runtest3

#:! tclsh %

tcl_exit 0
