/*
  Javascript wrapper for the Minsky WebAssembly module (jsminsky.js),
  adapted from the Ravel wrapper (ravel.js).

  usage:
    const Module=await createMinsky();          // from jsminsky.js
    const minsky=newMinsky(Module,'canvasId');  // a <canvas> element's id
    minsky.load('/model.mky');                  // files live in Module.FS
    minsky.canvas.zoomToFit();
    minsky.redraw();

  Every method of the C++ Minsky object is available as a Javascript
  method with the same name: arguments are marshalled as JSON, and the
  return value is the JSON result. Where a call returns an object,
  the methods of that object are attached to the returned value, which
  is itself a function giving the object's value, ie

     minsky.variableValues().$keys()
     // methods beginning with @ are written with $, eg $keys
     minsky.variableValues()()                     // the whole value as an object

  Elements of maps are addressed by path, which can be called directly:

     minsky.call('variableValues.@elem.:Y.value')

  Errors thrown in C++ appear as Javascript Errors.
*/

// bind the members of the C++ object \a impl whose names start with
// \a prefix as methods of \a object
function attachMethods(impl, object, prefix, methods) {
  for (let i=0; i<methods.size(); ++i) {
    const fullMethod=methods.get(i);
    if (fullMethod.length>prefix.length && fullMethod.startsWith(prefix)) {
      const m=fullMethod.slice(prefix.length).replace(/@/g,'$');
      if (m.indexOf('.')<0) {
        // defineProperty, as plain assignment fails for names that
        // functions already have (eg 'arguments', 'caller', 'name', 'length')
        const method=(...args)=>{
          const resultObj=impl.call(fullMethod,JSON.stringify(args));
          const children=resultObj.list();
          if (children.size()>0) {
            // an object: return a function giving its value (converted
            // on demand, as it can be large), with its methods attached
            const r=()=>parseJSON(resultObj.json());
            attachMethods(resultObj,r,"",children);
            return r;
          }
          return parseJSON(resultObj.json());
        };
        Object.defineProperty(object,m,{value: method, writable: true, configurable: true, enumerable: true});
        attachMethods(impl,method,fullMethod+'.',methods);
      }
    }
  }
}

// Minsky emits NaN and Infinity, which are not strict JSON
function parseJSON(s) {
  try {
    return JSON.parse(s);
  } catch (e) {
    return JSON.parse(s.replace(/\bNaN\b/g,'null').replace(/-?\bInfinity\b/g,'null'));
  }
}

const ZOOM_IN_FACTOR=1.1, ZOOM_OUT_FACTOR=0.91;

function newMinsky(Module, canvasId, options={}) {
  const canvasElem=typeof canvasId==="string"? document.getElementById(canvasId): canvasId;
  const ctx=canvasElem.getContext('2d');
  const impl=new Module.Minsky(ctx);
  const minsky=()=>{};
  minsky.impl=impl;
  attachMethods(impl,minsky,"",impl.list());
  // call the method at \a path, with the remaining arguments
  minsky.call=(path,...args)=>parseJSON(impl.call(path,JSON.stringify(args)).json());
  minsky.onRedraw=()=>{}; // hook function called after each redraw

  // redraws are coalesced into one per animation frame
  let pending=false;
  minsky.redraw=()=>{
    ctx.setTransform(1,0,0,1,0,0);
    impl.render(canvasElem.width,canvasElem.height);
    minsky.onRedraw();
  };
  minsky.requestRedraw=()=>{
    if (pending) return;
    pending=true;
    requestAnimationFrame(()=>{pending=false; minsky.redraw();});
  };

  // read a file into the module's virtual file system, so it can be loaded
  minsky.writeFile=(path,data)=>Module.FS.writeFile(path,
      data instanceof ArrayBuffer? new Uint8Array(data): data);
  // fetch a model from a URL, and load it
  minsky.loadURL=async (url)=>{
    const response=await fetch(url);
    if (!response.ok) throw new Error(url+": "+response.statusText);
    minsky.writeFile('/model.mky',await response.arrayBuffer());
    minsky.load('/model.mky');
    minsky.canvas.zoomToFit();
    minsky.redraw();
  };

  const pos=event=>{
    const r=canvasElem.getBoundingClientRect();
    // account for CSS scaling of the canvas
    return [(event.clientX-r.left)*canvasElem.width/r.width,
            (event.clientY-r.top)*canvasElem.height/r.height];
  };
  const guarded=f=>(...a)=>{
    try {f(...a);} catch (e) {(options.onError||console.error)(e);}
    minsky.requestRedraw();
  };

  canvasElem.onmousedown=guarded(event=>{
    if (event.button!==0) return;
    const [x,y]=pos(event);
    if (event.ctrlKey || event.metaKey) minsky.canvas.controlMouseDown(x,y);
    else minsky.canvas.mouseDown(x,y);
  });
  canvasElem.onmouseup=guarded(event=>{
    if (event.button!==0) return;
    minsky.canvas.mouseUp(...pos(event));
  });
  canvasElem.onmousemove=guarded(event=>{
    minsky.canvas.mouseMove(...pos(event));
  });
  canvasElem.onwheel=guarded(event=>{
    if (event.deltaY) {
      const [x,y]=pos(event);
      minsky.canvas.zoom(x,y,event.deltaY<0? ZOOM_IN_FACTOR: ZOOM_OUT_FACTOR);
    }
    event.preventDefault();
  });
  // keyboard focus is needed to receive key events
  canvasElem.tabIndex=canvasElem.tabIndex>=0? canvasElem.tabIndex: 0;
  canvasElem.onkeydown=guarded(event=>{
    if (event.key.length!==1 && !specialKeys[event.key]) return;
    const [x,y]=lastPos;
    minsky.canvas.keyPress({keySym: specialKeys[event.key] ?? event.key.codePointAt(0),
      utf8: event.key.length===1? event.key: "", state: modifiers(event), x, y});
    event.preventDefault();
  });
  let lastPos=[0,0];
  canvasElem.addEventListener('mousemove',event=>{lastPos=pos(event);});

  minsky.redraw();
  return minsky;
}

// X11 key syms for the non-printing keys the canvas responds to
const specialKeys={
  Backspace: 0xff08, Tab: 0xff09, Enter: 0xff0d, Escape: 0xff1b, Delete: 0xffff,
  ArrowLeft: 0xff51, ArrowUp: 0xff52, ArrowRight: 0xff53, ArrowDown: 0xff54,
};
const modifiers=e=>(e.shiftKey? 1: 0)|(e.ctrlKey? 4: 0)|(e.altKey? 8: 0);

if (typeof module!=="undefined") module.exports={newMinsky, attachMethods, parseJSON};
