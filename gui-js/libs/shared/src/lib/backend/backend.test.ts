import {describe, expect, test} from '@jest/globals';
import {minsky} from './minsky'

describe('properties',()=>{
  test('get-set',async ()=>{
    let m=await minsky.properties();
    expect("t" in m).toBe(true);
    m.t+=1;
    let t=await minsky.t(m.t);
    expect(t).toBe(m.t);
    let p=await minsky.properties(m);
    expect(p.t).toBe(m.t);
    p=await minsky.properties();
    expect(p.t).toBe(m.t);
    await new Promise((r)=>setTimeout(r,100)); // pause a bit to allow initialisation to complete
  });
});

         
