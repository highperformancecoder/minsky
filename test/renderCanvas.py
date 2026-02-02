#!/usr/bin/python3
import sys

# a crummy little utility for dumping the rendered canvases as a svg files
from pathlib import Path
here=str(Path(sys.argv[0]).parent.parent)
if here=="": here='.' #relative path
sys.path.insert(0,here)

# work out where the assets are
assetDir=here+'/gui-js/apps/minsky-web/src/assets/images/icons/'

from pyminsky import minsky
minsky.defaultFont('Sans')
minsky.setGodleyIconResource(assetDir+'bank.svg')
minsky.setGroupIconResource(assetDir+'group.svg')
minsky.srand(10)
minsky.load(sys.argv[1])
minsky.renderCanvasToSVG(sys.argv[2])

