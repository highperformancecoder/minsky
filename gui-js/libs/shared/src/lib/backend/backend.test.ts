import {describe, expect, test} from '@jest/globals';
import {minsky} from './minsky'

describe('properties',()=>{
  test('get-set',async ()=>{
    let m=await minsky.$properties();
    expect("t" in m).toBe(true);
    m.t+=1;
    //expect(await minsky.t(m.t)).toBe(m.t);
    //m.t+=1;
    let p=await minsky.$properties(m);
    expect(p.t).toBe(m.t);
    expect(await minsky.t()).toBe(m.t);
    p=await minsky.$properties();
    expect(p.t).toBe(m.t);
//    await new Promise((r)=>setTimeout(r,1000)); // pause a bit to allow initialisation to complete
  });
});

         
