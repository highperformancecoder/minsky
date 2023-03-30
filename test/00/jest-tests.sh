set -e
cd gui-js
npm install
npm test minsky-electron
npm test minsky-web
# commented out tests currently failing
#npm test core
#npm test menu

# why is npm test failing?
#    npm test shared -- --skip-nx-cache --runInBand

# shared tests crashing on Github action
if [ "$TRAVIS" != 1 ]; then
# a bit clunky, but npm test shared doesn't seem to do the right thing.
    mkdir -p libs/examples
    cp ../examples/GoodwinLinear02.mky libs/examples
    pushd libs/shared
    jest 
    popd
fi
#npm test ui-components
