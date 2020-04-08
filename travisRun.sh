#!/bin/bash
set -e
git submodule update --init --recursive

pushd ecolab
make -j2 all-without-models
popd

make -j2 DEBUG=1
export TRAVIS=1
# This step generates reference image files on the CI server 
#  - sh createRenderedEquations
xvfb-run make -j2 AEGIS=1 sure
# currently failing on Travis due to OpenSUSE Tumbleweed bug. Ticket #1170
#make doc
