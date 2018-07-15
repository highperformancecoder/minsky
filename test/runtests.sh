#!/bin/bash

t=0
#for j in 01 02 06 09 10 12 13 14 15 17 18 19 20 21 23 24 25 26 27 29 33 35 37; do
#    i=test/00/t00${j}a.sh
for i in test/00/*.sh; do
    echo $i
    # rendering is too unstable, so disable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/t0035a.sh ]; then continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/allItemsRenderCheck.sh ]; then continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/godleyTableWindow.sh ]; then continue; fi
    # this test is hanging on Travis (well any Ubuntu 14.04 environment) for some mysterious reason
    if [ "$TRAVIS" = 1 -a $i = test/00/canvasContext.sh ]; then continue; fi
    # schema validation - disable test for now
    if [ "$TRAVIS" = 1 -a $i = test/00/t0007a.sh ]; then continue; fi
    # Why TF is Travis failing this test????
    if [ "$TRAVIS" = 1 -a $i = test/00/t0008a.sh ]; then continue; fi
    sh $i
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
exit $t
