// Smoke test of the Minsky wasm module against a mock canvas context.
// usage: node test/smoke.js  (from the wasm directory, or: make test)
const assert=require('assert');
const fs=require('fs');
const path=require('path');
const createMinsky=require('../jsminsky.js');
const {makeMockContext}=require('./mockCanvas.js');

const names=w=>{const l=w.list(); const r=[]; for (let i=0; i<l.size(); ++i) r.push(l.get(i)); return r;};

createMinsky().then(M=>{
  const {ctx,log}=makeMockContext(800,600);
  const m=new M.Minsky(ctx);

  // API discovery
  const api=names(m);
  assert(api.length>100,'expected a large API, got '+api.length);
  assert(api.some(n=>n.startsWith('canvas')),'no canvas API');
  console.log('API size',api.length);
  assert(/\d/.test(m.call('minskyVersion','[]').json()));

  // C++ exceptions surface as Javascript errors with a message
  assert.throws(()=>m.call('load','["/no/such/file.mky"]'), /./);
  assert.throws(()=>m.call('noSuchCommand','[]'), /not found/i);

  // an empty canvas renders
  m.render(800,600);
  assert(log.length>0,'nothing drawn');

  // load a model and simulate. The reference values are those of the
  // native build (pyminsky), which this must reproduce exactly.
  const mky=path.join(__dirname,'../../examples/GoodwinLinear.mky');
  M.FS.writeFile('/g.mky',fs.readFileSync(mky));
  m.call('load','["/g.mky"]');
  m.call('reset','[]');
  for (let i=0; i<50; ++i) m.call('step','[]');
  assert.strictEqual(m.call('t','[]').json(),'0.34600000000000025');
  assert.strictEqual(m.call('variableValues.@elem.:Y.value','[]').json(),'100.99024361459334');

  // the loaded model draws items: text, paths and fills
  log.length=0;
  m.render(800,600);
  const count=p=>log.filter(l=>l.startsWith(p)).length;
  console.log('canvas calls for GoodwinLinear:',log.length,
              'fillText',count('fillText'),'stroke',count('stroke'),'fill',count('fill'));
  assert(count('fillText')>5,'variable names not drawn');
  assert(count('stroke')>5,'no wires/outlines drawn');
  assert.strictEqual(count('save'),count('restore'),'unbalanced save/restore');

  // the Javascript wrapper mirrors the C++ object structure
  const {attachMethods}=require('../minsky.js');
  const minsky=()=>{};
  attachMethods(m,minsky,'',m.list());
  assert.strictEqual(minsky.t(),0.34600000000000025);
  assert(minsky.variableValues().$keys().includes(':Y'));
  assert.strictEqual(m.call('variableValues.@elem.:Y.value','[]').json(),'100.99024361459334');
  // events: pick up the item under the pointer, drag it, and check the model changed
  minsky.canvas.zoomToFit();
  minsky.canvas.mouseDown(400,300);
  minsky.canvas.mouseMove(410,310);
  minsky.canvas.mouseUp(410,310);
  minsky.canvas.zoom(400,300,1.1);
  assert(minsky.canvas.zoomFactor()>0);
  log.length=0;
  m.render(800,600);
  assert(log.length>0);
  // adding an operation changes what is drawn
  m.call('clearAllMaps','[]');
  m.call('load','["/g.mky"]');
  log.length=0; m.render(800,600);
  const before=log.length;
  minsky.canvas.addOperation('add');
  log.length=0; m.render(800,600);
  assert(log.length>before,'adding an operation should draw more: '+before+' -> '+log.length);
  console.log('smoke.js: all passed');
}).catch(e=>{console.error(e); process.exit(1);});
