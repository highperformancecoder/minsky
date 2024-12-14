#!/usr/bin/bash
MINSKY_VERSION=Minsky-`git describe`
git archive --format=tar --prefix=$MINSKY_VERSION/ HEAD -o /tmp/$MINSKY_VERSION.tar

# package node headers. Pass node header path as arg 1
if [ $# -ge 1 ]; then
	rm -rf /tmp/$$
	mkdir -p /tmp/$$/$MINSKY_VERSION/node_modules
	cp -r node_modules/node-addon-api /tmp/$$/$MINSKY_VERSION/node_modules
	cp -r $1/* /tmp/$$/$MINSKY_VERSION/node_modules/node-addon-api
	tar rf /tmp/$MINSKY_VERSION.tar -C /tmp/$$ $MINSKY_VERSION
fi

# add in submodules
for sub in ecolab ecolab/classdesc ecolab/classdesc/json5_parser ecolab/graphcode RavelCAPI RavelCAPI/civita exprtk; do
    pushd $sub
    git archive --format=tar --prefix=$MINSKY_VERSION/$sub/ HEAD -o /tmp/$$.tar
    tar tvf /tmp/$$.tar
    tar Af /tmp/$MINSKY_VERSION.tar /tmp/$$.tar
    rm /tmp/$$.tar
    popd
done
gzip -f /tmp/$MINSKY_VERSION.tar
