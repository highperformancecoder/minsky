# used for benchmarking optimisation of the reset() function

from sys import argv
from pyminsky import minsky
from timeit import timeit
from os import path
try:
    minsky.load(argv[1])
except:
    pass

def reset():
    try:
        minsky.reset()
    except:
        pass

print(path.basename(argv[1]),timeit(reset,globals=globals(),number=1),sep=',')
