# used for benchmarking optimisation of the reset() function

from sys import argv
from pyminsky import minsky
from timeit import timeit
try:
    minsky.load(argv[1])
except:
    pass

def reset():
    try:
        minsky.reset()
    except:
        pass

print(argv[1],timeit(reset,globals=globals(),number=1),sep=',')
