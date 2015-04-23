#!server

# example of minskyConnect contents is
#   set connection "postgresql://dbname=minsky user=xxx password=yyy"
# or
#   set connection "mysql://db=minsky user=xxx password=yyy"
source $env(HOME)/minskyConnect.tcl
databaseServer.openDb $connection

#if argv(1) has .tcl extension, it is a script, otherwise it is data
if {$argc>1} {
    if [string match "*.tcl" $argv(1)] {
	    source $argv(1)
	} else {
	    databaseServer.load $argv(1)
            tcl_exit
	}
}

databaseServer.listenerThreads 0
databaseServer.workerThreads 0
databaseServer.port 8000
databaseServer.start
