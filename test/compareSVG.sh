#!/bin/bash
# render SVG files to PNM, stripping all metadata, and comparing for pixel equality
mkdir tmpImages-$$
rsvg-convert $1 |pngtopnm >tmpImages-$$/1.pnm
rsvg-convert $2 |pngtopnm >tmpImages-$$/2.pnm
diff tmpImages-$$/1.pnm tmpImages-$$/2.pnm
status=$?
rm -rf tmpImages-$$
exit $status
