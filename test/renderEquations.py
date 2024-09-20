import sys
from pathlib import Path
here=Path(sys.argv[0]).parent.parent
if here=="": here='.' #relative path
sys.path.append(str(here))

from pyminsky import minsky
minsky.defaultFont('Sans')
minsky.load(sys.argv[1])
minsky.equationDisplay.renderToSVG(sys.argv[1]+'.svg')

