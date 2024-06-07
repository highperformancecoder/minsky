#!/bin/bash

pdflatex -interaction=batchmode Ravel
if [ $? -ne 0 ]; then exit 1; fi

pdflatex -interaction=batchmode Ravel
latex2html -local_icons -white -info "" -html_version math -contents Ravel.html Ravel
if [ $? -ne 0 ]; then exit 1; fi

# copy images, and mutate table of contents after latex2html suitable
# for our in house help file style

cp -f images/*.png Ravel
sed -i -e 's/^\(  HREF=[^>]*\)>/\1 target="content">/' Ravel/Ravel.html

#check for any broken links
# install via: pip3 install git+https://github.com/linkchecker/linkchecker.git
linkchecker -f ../linkcheckerrc minsky.html
exit $?
