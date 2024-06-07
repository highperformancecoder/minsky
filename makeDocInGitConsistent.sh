#!/bin/bash
rm -rf doc/Ravel
(cd doc; sh makedoc.sh)
git add doc/Ravel.pdf

target=gui-js/minsky-docs
git rm -rf $target/Ravel/*
mkdir -p $target/Ravel
find doc/Ravel \( -name "*.html" -o -name "*.css" -o -name "*.png" \) -exec cp {} $target/Ravel \;
cp -r -f doc/minsky.html $target
for i in $target/Ravel/*; do git add $i; done
git add $target/minsky.html

git commit -m "Making doc consistent in git"
