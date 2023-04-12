#!/bin/bash

# remove any backup files to prevent unintended hangs
rm examples/*.mky# test/*.mky#

t=0
for i in test/00/*.sh; do
    echo -n "$i: "
    # rendering is too unstable, so disable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/renderEquations.sh ]; then echo disabled; continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/renderItemTab.sh ]; then echo disabled; continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/allItemsRenderCheck.sh ]; then echo disabled; continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/godleyTableWindow.sh ]; then echo disabled; continue; fi
    # greek characters in variable names cause problems in docker containers
    if [ "$TRAVIS" = 1 -a $i = test/00/compareSimulationWithLogged.sh ]; then echo disabled; continue; fi
    # disable RESTProcess for now
#    if [ $i = test/00/RESTService.sh ]; then echo disabled; continue; fi
    # we need to expose output generated here to prevent a build timeout
    if [ "$TRAVIS" = 1 -a $i = test/00/checkOverrides.sh ]; then
        sh $i
#    elif [ "$TRAVIS" = 1 -a $i = test/00/jest-tests.sh ]; then
#        sh $i
    # checkReadOnlySaveFails tests that saving over a readonly file is prevented. Test doesn't work as root
    elif [ `whoami` = 'root' -a $i = test/00/checkReadOnlySaveFails.sh ]; then
        su minsky $i &> /dev/null
    else
        sh $i &> /dev/null
    fi
    if test $? -eq 0 ; then 
        echo passed
    else
        let $[t++]
        echo -e "\033[31mfailed\033[0m"
    fi
done
exit $t
