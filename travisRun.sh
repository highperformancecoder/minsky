#!/bin/bash
set -e

# force build ecolab, as it needs to be built before the .d files are generated
# not a problem in a usual dev environment, with classdesc installed
(cd ecolab; make -j2 AEGIS=1 $DEBUG all-without-models)
make -j2 AEGIS=1 $DEBUG OBS=1
export TRAVIS=1
(cd gui-js && yes|npm install) 
#make -j2 AEGIS=1 sure
make -j2 AEGIS=1 tests
make doc
