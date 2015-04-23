#!/bin/bash

latex -interaction=batchmode minsky
if [ $? -ne 0 ]; then exit 1; fi

latex -interaction=batchmode minsky
latex2html -info "" minsky
if [ $? -ne 0 ]; then exit 1; fi

# copy images, and mutate table of contents after latex2html suitable
# for our in house help file style

cp -f images/* minsky
sed -e 's/^\(  HREF=[^>]*\)>/\1 target="content">/' <minsky/minsky.html >tmp
mv tmp minsky/minsky.html

#check for any broken links
linkchecker minsky.html

