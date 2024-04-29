import sys
from pathlib import Path
here=str(Path(sys.argv[0]).parent)
if here: sys.path.append(here)
else: sys.path.append('.')

from pyminsky import minsky

iconDir=here+'/gui-js/apps/minsky-web/src/assets/images/icons/'
minsky.histogramResource.setResource(iconDir+'histogram.svg')
# make the background transparent
minsky.canvas.backgroundColour({'a':0})
ops=minsky.availableOperations()
ops.append("switch")
for op in ops:
    # ignore some operations
    if op in ["numOps","constant","copy","ravel","integrate"]: continue

    if op=="switch": minsky.canvas.addSwitch()
    else: minsky.canvas.addOperation(op)
    minsky.renderCanvasToPNG(iconDir+op+'.png')
    minsky.clearAllMaps()
