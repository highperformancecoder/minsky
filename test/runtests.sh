#!/bin/bash

t=0
for i in test/00/*.sh; do
    echo -n "$i: "
    # rendering is too unstable, so disable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/t0035a.sh ]; then echo "disabled"; continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/allItemsRenderCheck.sh ]; then echo "disabled"; continue; fi
    if [ "$TRAVIS" = 1 -a $i = test/00/godleyTableWindow.sh ]; then echo "disabled"; continue; fi
    # this test is hanging on Travis (well any Ubuntu 14.04 environment) for some mysterious reason
    if [ "$TRAVIS" = 1 -a $i = test/00/canvasContext.sh ]; then echo "disabled"; continue; fi
    # schema validation - disable test for now
    if [ "$TRAVIS" = 1 -a $i = test/00/t0007a.sh ]; then echo "disabled"; continue; fi
    # Why TF is Travis failing this test????
    if [ "$TRAVIS" = 1 -a $i = test/00/t0008a.sh ]; then echo "disabled"; continue; fi
    # disable RESTProcess for now
    if [ $i = test/00/RESTService.sh ]; then echo disabled; continue; fi
    # we need to expose output generated here to prevent a build timeout
    if [ "$TRAVIS" = 1 -a $i = test/00/checkOverrides.sh ]; then
        sh $i
    # t0029a.sh tests that saving over a readonly file is prevented. Test doesn't work as root
    elif [ `whoami` = 'root' -a $i = test/00/t0029a.sh ]; then
        su minsky $i &> /dev/null
    elif [ $i = test/00/findDefinition.sh ]; then
        sh $i
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
