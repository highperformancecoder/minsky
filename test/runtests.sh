#!/bin/bash

t=0
#for i in 01 02 03 04 05 06 09 10 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37; do 
for i in test/00/*.sh; do
    if [ $i = "test/00/t0008a.sh" ]; then continue; fi # TODO schema 0 support
    # rendering to GIFs is too unstable, so diable on the Travis platform
    if [ "$TRAVIS" = 1 && $i = test/00/t0035a.sh ]; then continue; fi
    sh $i
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
exit $t
