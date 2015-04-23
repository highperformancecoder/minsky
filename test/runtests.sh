#!/bin/bash

t=0
for i in test/00/*.sh; do 
  sh $i
   t=$[$t+$?]
done
exit $t
