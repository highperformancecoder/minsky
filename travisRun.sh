#!/bin/bash
set -e
rm -rf ecolab
git clone https://github.com/highperformancecoder/ecolab.git
pushd ecolab
git submodule init
git submodule update
make -j2 install
popd

echo HOME=$HOME

make -j2 DEBUG=1
export TRAVIS=1
# This step generates reference image files on the CI server 
#  - sh createRenderedEquations
xvfb-run make -j2 AEGIS=1 sure
make doc
