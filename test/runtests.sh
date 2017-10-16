#!/bin/bash

t=0
#for j in 01 02 06 09 10 12 13 14 15 17 18 19 20 21 23 24 25 26 27 29 33 35 37; do
#    i=test/00/t00${j}a.sh
for i in test/00/*.sh; do
    echo $i
    if [ $i = "test/00/t0008a.sh" ]; then continue; fi # TODO schema 0 support
    if [ $i = "test/00/t0005a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0022a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0028a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0030a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0031a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0032a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0034a.sh" ]; then continue; fi 
    if [ $i = "test/00/t0036a.sh" ]; then continue; fi 
    # rendering is too unstable, so disable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/t0035a.sh ]; then continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/allItemsRenderCheck.sh ]; then continue; fi
    sh $i
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
exit $t
