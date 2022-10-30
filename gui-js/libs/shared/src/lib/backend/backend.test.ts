import {describe, expect, test} from '@jest/globals';
import {minsky} from './minsky';

describe('properties',()=>{
  test('get-set',()=>{
    let m=minsky.properties();
    expect("t" in m).toBe(true);
    m.t+=1;
    expect(minsky.properties(m).t).toBe(m.t);
    expect(minsky.properties().t).toBe(m.t);
  });
});

         
