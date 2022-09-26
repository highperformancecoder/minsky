import {describe, expect, test} from '@jest/globals';
import {
  minsky,
} from './backend';

describe('Minsky', ()=>{
  test('save empty',()=>{
    minsky.save("/tmp/foo.mky");
  });
  test('load empty', ()=>{
    minsky.load("/tmp/foo.mky");
  });
});
