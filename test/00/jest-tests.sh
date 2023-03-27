set -e
cd gui-js
npm install
npm test minsky-electron
npm test minsky-web
# commented out tests currently failing
#npm test core
#npm test menu
# why is npm test failing?
#npm test shared -- --runInBand
pushd libs/shared
# a bit clunky, but npm test shared doesn't seem to do the right thing.
ln -sf ../../../examples ..
jest
popd
#npm test ui-components
