#!/bin/bash

for i in GUI/*.tcl; do
  test/tcl-cov minsky.cov $i >$i.cov
done
