import {describe, expect, test} from '@jest/globals';
import {minsky} from './minsky';
import { restService } from './backend';

// rewrite message callbacks to avoid dereferencing a dialog
restService.setMessageCallback((msg: string, buttons: string[])=> {
  if (msg) console.log(msg);
  return 0;
});

restService.setBusyCursorCallback((busy: boolean)=>{});

describe('Minsky load/save', ()=>{
  test('save empty',()=>{
    minsky.save("/tmp/foo.mky");
  });
  test('load empty', ()=>{
    minsky.load("/tmp/foo.mky");
  });
});

describe('Named Items',()=>{
  test('name item', ()=>{
    minsky.canvas.addOperation("time");
    minsky.canvas.getItemAt(0,0);
    minsky.nameCurrentItem("foo");
    expect(minsky.namedItems.elem("foo").second.classType()).toBe("Operation:time");
  });
});
