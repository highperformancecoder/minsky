#!/bin/bash

for i in gui-tk/*.tcl; do
  test/tcl-cov minsky.cov $i >$i.cov
done
