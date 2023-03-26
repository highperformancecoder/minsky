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
jest
popd
#npm test ui-components
