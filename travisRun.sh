#!/bin/bash
set -e

make -j2 AEGIS=1 DEBUG=1
export TRAVIS=1
make -j2 AEGIS=1 sure
#make -j2 AEGIS=1 tests
make doc
