#!/usr/bin/bash
# for benchmarking a series of files for reset optimisation
for i in $*; do
    python3 resetRavel.py $i
    if [ $? -ne 0 ]; then
        echo "$i failed"
    fi
done
