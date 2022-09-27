import {describe, expect, test} from '@jest/globals';
import {
  minsky,
  restService,
} from './backend';

// rewrite message callabase to avoid dereferencing a dialog
restService.setMessageCallback(function (msg: string, buttons: string[]) {
  if (msg) console.log(msg);
  return 0;
});


describe('Minsky', ()=>{
  test('save empty',()=>{
    minsky.save("/tmp/foo.mky");
  });
  test('load empty', ()=>{
    minsky.load("/tmp/foo.mky");
  });
});
