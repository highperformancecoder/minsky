import sys
sys.path.append('.')
from pyminsky import minsky
minsky.load(sys.argv[1])
minsky.garbageCollect()
minsky.save(sys.argv[2])
