import sys
import json
from pyminsky import minsky, DataSpec
minsky.databaseIngestor.connect("sqlite3","db=citibike.sqlite")
# sys.argv[0] is this script name
filenames=sys.argv[1:]

# set up spec for Citibike
spec=DataSpec()
spec.setDataArea(1,14)
spec.dataCols([0,11,14])
spec.dimensionCols([1,4,8,12,13])
spec.dimensionNames(["tripduration","starttime","stoptime","start station id","start station name","start station latitude","start station longitude","end station id","end station name","end station latitude","end station longitude","bikeid","usertype","birth year","gender"])
spec.dimensions(15*[{"type":"string","units":""}])
spec.dimensions[1]({"type":"time","units":""})
spec.dimensions[11]({"type":"value","units":""})
spec.dimensions[14]({"type":"value","units":""})
spec.duplicateKeyAction("av")
spec.numCols(15)

#'{"counter":false,"dataColOffset":14,"dataCols":[0,11],"dataRowOffset":1,"decSeparator":".","dimensionCols":[1,4,8,12,13,14],"dimensionNames":["tripduration","starttime","stoptime","start station id","start station name","start station latitude","start station longitude","end station id","end station name","end station latitude","end station longitude","bikeid","usertype","birth year","gender"],"dimensions":[{"type":"value","units":""},{"type":"time","units":""},{"type":"time","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""},{"type":"string","units":""}],"dontFail":false,"duplicateKeyAction":"av","escape":"\0","headerRow":0,"horizontalDimName":"?","horizontalDimension":{"type":"string","units":""},"maxColumn":1000,"mergeDelimiters":false,"missingValue":NaN,"numCols":15,"quote":"\"","separator":","})'

minsky.databaseIngestor.table("citibike")
minsky.databaseIngestor.createTable(filenames,spec())
minsky.databaseIngestor.importFromCSV(filenames,spec())
