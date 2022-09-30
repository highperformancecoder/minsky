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

//describe('addIntegral',()=>{
//  test('addIntegral', ()=>{
//    minsky.clearAllMaps();
//    minsky.addIntegral();
//    minsky.canvas.getItemAt(0,0);
//    expect(minsky.canvas.item.classType()).toBe("IntOp");
//  });
//});

describe('operations',()=>{
  test('operations', ()=>{
    expect(minsky.availableOperations()).toEqual(expect.arrayContaining(["time"]));
    expect(minsky.classifyOp("time")).toBe("general");
  });
});

describe('dirty flag',()=>{
  test('edited', ()=>{
    minsky.clearAllMaps();
    expect(minsky.edited()).toBe(false);
    minsky.addIntegral();
    expect(minsky.edited()).toBe(true);
    minsky.clearAllMaps();
    expect(minsky.edited()).toBe(false);
  });
});

describe('clipboard',()=>{
  test('copy/cut/paste',()=>{
    minsky.load("../examples/GoodwinLinear02.mky");
    minsky.canvas.mouseDown(0,0);
    minsky.canvas.mouseUp(200,200);
    minsky.copy();
    expect(minsky.canvas.selection.empty()).toBe(false);
    let numItems=minsky.model.items.size();
    let numSelected=minsky.canvas.selection.items.size();
    expect(numItems).toBeGreaterThan(0);
    expect(numSelected).toBeGreaterThan(0);
    minsky.cut();
    expect(minsky.model.items.size()+numSelected).toBe(numItems);
    minsky.paste();
    expect(minsky.model.items.size()).toBe(numItems);
  });
});
  
