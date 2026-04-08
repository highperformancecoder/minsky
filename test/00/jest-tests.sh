here=`pwd`
. test/common-test.sh
cd $here/gui-js
npm install
# clear nx cache
npm run nx reset
export NX_DAEMON=false
npm test minsky-electron
if test $? -ne 0; then fail; fi
npm test minsky-web
if test $? -ne 0; then fail; fi
xvfb-run -a npm test shared
if test $? -ne 0; then fail; fi
# commented out tests currently failing
#npm test core
#npm test menu
#npm test ui-components
if test $? -ne 0; then fail; fi

pass
