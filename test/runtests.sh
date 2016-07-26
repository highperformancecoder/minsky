#!/bin/bash

t=0
for i in 01 03 04 05 06 09 10 12 13 14 15 16 17; do 
  sh test/00/t00${i}a.sh
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
exit $t
