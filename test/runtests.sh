#!/bin/bash

t=0
for j in 01 02 03 05 06 09 10 12 13 14 15 16 17 18 19 20 21 23 24 25 26 27 29 32 33 35 36 37; do
    i=test/00/t00${j}a.sh
#for i in test/00/*.sh; do
    if [ $i = "test/00/t0008a.sh" ]; then continue; fi # TODO schema 0 support
    # rendering to GIFs is too unstable, so disable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/t0035a.sh ]; then continue; fi
    sh $i
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
exit $t
