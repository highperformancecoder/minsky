#!/usr/bin/python3

# usage python3 compareFileLog.py <system.mky> <logfile>

# compares the running of a minsky model <system.mky>, with a previously
# run data log file <logfile>, created by "createFileLog"

import sys
import csv
import re

from pathlib import Path
here=Path(sys.argv[0]).parent.parent
if here=="": here='.' #relative path
sys.path.append(str(here))

from pyminsky import minsky

def fclose(x,y):
    if abs(x)>1e-30:
        return abs(x-y)/(abs(x)+abs(y)) < 1e-2
    return abs(x-y) <= 1e-30

minsky.load(sys.argv[1])
        
ret=0

minsky.running(True)
#provide seed to ensure repeatability
minsky.srand(10)
minsky.reset()

valueDict={}
def testValue(key, value):
    # check that any local variable of the same name matches out value
    if not any([fclose(value, x) for x in valueDict[key]]):
        print(sys.argv[1], "t=",minsky.t(),name," logged ",value)
        ret=1

with open(sys.argv[2]) as log:
    reader=csv.reader(log)
    # first line should specify no of steps to run
    row=next(reader)
    nsteps=int(row[1])

    for step in range(nsteps):
        row=next(reader)
        minsky.step()

        if not fclose(minsky.t(),float(row[0])):
            print("t=",minsky.t()," logged ",row[0])
            ret=1
        
        
        for i in range(1,len(row),2):
            # stash these in an array of values, to represent multiple same named local variables
            if row[i] in valueDict:
                valueDict[row[i]].append(float(row[i+1]))
            else:
                valueDict[row[i]]=[float(row[i+1])]


        for name in minsky.variableValues.keys():
            if re.search("^constant:",name) != None: continue
            value=minsky.variableValues[name]
            if value.temp(): continue

            if value.size()>1:
                for i in range(value.size()):
                    testValue(f"{value.name()}|{i}",value.value())
            else:
                testValue(value.name(),value.value())

exit(ret)
