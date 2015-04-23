puts "TCL_LIB=[set tcl_library]"
regsub tcl [info library] tk tk_library
puts "TK_LIB=[set tk_library]"
