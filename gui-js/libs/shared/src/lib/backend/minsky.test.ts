import {describe, expect, test} from '@jest/globals';
import {minsky} from './minsky';
import {Variable} from './variable';

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

describe('addIntegral',()=>{
  test('addIntegral', ()=>{
    minsky.clearAllMaps();
    expect(minsky.model.items.size()).toBe(0);
    minsky.canvas.addVariable("foo","flow");
    expect(minsky.model.items.size()).toBe(1);
    minsky.canvas.getItemAt(0,0);
    expect(minsky.canvas.item.classType()).toBe("Variable:flow");
    expect((new Variable(minsky.canvas.item)).name()).toBe("foo");
    minsky.addIntegral();
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.model.items.elem(1).classType()).toBe("IntOp");
  });
});

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
    minsky.canvas.addOperation("time");
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
  
describe('history',()=>{
  test('undo',()=>{
    minsky.clearAllMaps();
    expect(minsky.undo(0)).toBe(1);
    minsky.canvas.addOperation("time");
    expect(minsky.undo(0)).toBe(2);
    expect(minsky.model.items.size()).toBe(1);
    minsky.canvas.addOperation("time");
    expect(minsky.undo(0)).toBe(3);
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.undo(1)).toBe(2);
    expect(minsky.model.items.size()).toBe(1);
    expect(minsky.undo(-1)).toBe(3);
    expect(minsky.model.items.size()).toBe(2);
  });
  test('doPushHistory',()=>{
    minsky.clearAllMaps(true);
    expect(minsky.undo(0)).toBe(1);
    expect(minsky.doPushHistory(false)).toBe(false);
    expect(minsky.doPushHistory()).toBe(false);
    minsky.canvas.addOperation("time");
    minsky.canvas.addOperation("time");
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.undo(1)).toBe(1);
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.doPushHistory(true)).toBe(true);
    expect(minsky.doPushHistory()).toBe(true);
    minsky.canvas.addOperation("time");
    expect(minsky.undo(0)).toBe(2);
    minsky.canvas.addOperation("time");
    expect(minsky.model.items.size()).toBe(4);
    expect(minsky.undo(0)).toBe(3);
    expect(minsky.undo(1)).toBe(2);
    expect(minsky.model.items.size()).toBe(3);
  });
  test('clearHistory',()=>{
    minsky.canvas.addOperation("time");
    minsky.canvas.addOperation("time");
    expect(minsky.undo(0)).toBeGreaterThan(1);
    minsky.clearHistory();
    expect(minsky.undo(0)).toBe(1);
  });
});
