import sys
import json
sys.path.insert(0,'.')
from pyminsky import minsky, DataSpec
minsky.databaseIngestor.db.connect("sqlite3","db=citibike.sqlite","citibike")
# sys.argv[0] is this script name
filenames=sys.argv[1:]

# set up spec for Citibike
spec=minsky.databaseIngestor.spec

spec.dataRowOffset(1)
spec.dataCols([0])
spec.dimensionCols([1,4,8,11,12,13,14])
spec.dimensionNames([
    "tripduration",
    "starttime",
    "stoptime",
    "start station id",
    "start station name",
    "start station latitude",
    "start station longitude",
    "end station id",
    "end station name",
    "end station latitude",
    "end station longitude",
    "bikeid",
    "usertype",
    "birth year",
    "gender"
])

spec.dimensions([
    {"type":"value"},
    {"type":"time"},
    {"type":"time"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"string"},
    {"type":"time"},
    {"type":"string"}
])
spec.dontFail(True)

minsky.databaseIngestor.db.createTable(filenames[0])
minsky.databaseIngestor.importFromCSV(filenames)
