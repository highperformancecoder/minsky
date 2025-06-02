import sys
import csv
import re
import argparse

from pathlib import Path
here=Path(sys.argv[0]).parent.parent
if here=="": here='.' #relative path
sys.path.append(str(here))

from argparse import ArgumentParser
parser=ArgumentParser(prog='compareWithOctave',description=
'Outputs data from a Minsky run, for input into compareWithOctave.m'
)
parser.add_argument('filename')
parser.add_argument('-i','--implicit',default=0,type=int)
parser.add_argument('-o','--order',default=4,type=int)
args=parser.parse_args(sys.argv[1:])

from pyminsky import minsky

minsky.load(args.filename)
minsky.reset()
minsky.matlab(args.filename+'.m')
minsky.order(args.order)
minsky.implicit(args.implicit)

# run the simulation for a while, and output the data values
with open(args.filename+'.dat', 'w') as dat:
    print("d=[",file=dat)
    minsky.running(True)
    for i in range(7):
        print("[", minsky.t(), end=' ',file=dat)
        for var in minsky.variableValues.keys():
            if not minsky.variableValues[var].isFlowVar():
                print(minsky.variableValues[var].value(), end=' ',file=dat)
        print("];",file=dat)
        minsky.step()
    print("];",file=dat)

