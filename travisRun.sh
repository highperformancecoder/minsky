#!/bin/bash
set -e

make -j2 AEGIS=1 DEBUG=1
export TRAVIS=1
xvfb-run make -j2 AEGIS=1 sure
#xvfb-run make -j2 AEGIS=1 tests
make doc
