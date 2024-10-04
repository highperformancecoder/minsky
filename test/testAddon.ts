const restService = require('bindings')('../gui-js/build/minskyRESTService.node');
// test synchronous version
console.log(restService.call("minsky.minskyVersion.$sync",""));

// test asynchronous call and return
setTimeout(async ()=>{
  console.log(await restService.call("minsky.minskyVersion",""));
  process.exit(0);
}, 1);

setTimeout(async ()=>{
  console.log('timed out');
  process.exit(1);
}, 10000);



           
