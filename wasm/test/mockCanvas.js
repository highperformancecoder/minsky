// A mock CanvasRenderingContext2D that records the calls made on it.
// It keeps just enough state (transform stack, text metrics) for
// MansouraHTMLCanvas to work against.
exports.makeMockContext=function(width=300,height=200)
{
  const log=[];
  const stack=[];
  const id={a:1,b:0,c:0,d:1,e:0,f:0};
  let tf={...id};
  const mock={
    getTransform(){return {...tf};},
    setTransform(a,b,c,d,e,f){tf={a,b,c,d,e,f};},
    resetTransform(){tf={...id};},
    translate(x,y){tf.e+=tf.a*x+tf.c*y; tf.f+=tf.b*x+tf.d*y;},
    scale(x,y){tf.a*=x; tf.b*=x; tf.c*=y; tf.d*=y;},
    save(){stack.push({...tf}); log.push('save');},
    restore(){tf=stack.pop(); log.push('restore');},
    measureText(t){
      const px=parseFloat(String(this.font).match(/([\d.]+)px/)[1]);
      return {width:t.length*0.5*px, fontBoundingBoxAscent:0.9*px, fontBoundingBoxDescent:0.2*px};
    },
    canvas:{width,height}, lineWidth:1, fillStyle:'#000000', strokeStyle:'#000000', font:'10px sans-serif',
  };
  const ctx=new Proxy(mock,{
    get(t,p){
      if (p in t) return t[p];
      return (...a)=>{log.push(String(p)+'('+a.join(',')+')');};
    },
    set(t,p,v){t[p]=v; return true;}
  });
  return {ctx,log};
};
