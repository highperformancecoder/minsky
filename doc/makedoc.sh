#!/bin/bash

pdflatex -interaction=batchmode minsky
if [ $? -ne 0 ]; then exit 1; fi

pdflatex -interaction=batchmode minsky
latex2html -local_icons -white -notransparent -info "" minsky
if [ $? -ne 0 ]; then exit 1; fi

# copy images, and mutate table of contents after latex2html suitable
# for our in house help file style

cp -f images/*.png minsky
sed -i -e 's/^\(  HREF=[^>]*\)>/\1 target="content">/' minsky/minsky.html

#check for any broken links
linkchecker -f ../linkcheckerrc minsky.html
echo $?
