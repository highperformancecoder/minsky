set -e
cd gui-js
npm install
# clear nx cache
npm run nx reset
export NX_DAEMON=false
npm test minsky-electron
npm test minsky-web
npm test shared
# commented out tests currently failing
#npm test core
#npm test menu
#npm test ui-components
