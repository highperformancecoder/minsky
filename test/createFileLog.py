#!/usr/bin/python3

# usage python3 createFileLog.py <system.mky>

# writes a log file on stdout for system <system.mky>

import sys
from pathlib import Path
here=Path(sys.argv[0]).parent.parent
if here=="": here='.' #relative path
sys.path.append(str(here))

from pyminsky import minsky
import re
minsky.load(sys.argv[1])


nSteps=10
print("nsteps,",nSteps)
minsky.running(True)
#provide seed to ensure repeatability
minsky.srand(10)
minsky.reset()
#step

for step in range(nSteps):
    minsky.step()
    print(minsky.t(),end='')

    for name in minsky.variableValues._keys():
        if re.search("^constant:",name) != None: continue
        value=minsky.variableValues[name]
        if value.temp(): continue
        if value.size()>1:
            for i in range(value.size()):
                print(f',"{value.name()}|{i}"',value.value(i),sep=',',end='')
        else:
            print(f',"{value.name()}"',value.value(),sep=',',end="")
    print()

