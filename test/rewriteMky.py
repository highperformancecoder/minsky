import sys
sys.path.insert(0,'.')
from pyminsky import minsky
minsky.load(sys.argv[1])
minsky.garbageCollect()
minsky.save(sys.argv[2])
