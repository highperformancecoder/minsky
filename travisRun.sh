#!/bin/bash
git clone https://github.com/highperformancecoder/ecolab.git
pushd ecolab
git submodule init
git submodule update
make CCACHE=1 install
popd

if [ $TEST_SUITE = sure ]; then 
    make DEBUG=1
    Xvfb :0 &>/dev/null &
    export DISPLAY=:0
    export MINSKY_TEST_DATABASE_PARAMS=$HOME/.testDatabase
    echo "testEq.mky" >$MINSKY_TEST_DATABASE_PARAMS
    psql -U postgres <server/minskyPG.sql
    echo "postgresql://dbname=postgres user=postgres password=">>$MINSKY_TEST_DATABASE_PARAMS
    export TRAVIS=1
# This step generates reference image files on the CI server 
#  - sh createRenderedEquations
    make CCACHE=1 AEGIS=1 sure
elif [ $TEST_SUITE = docs ]; then
    make doc
fi
