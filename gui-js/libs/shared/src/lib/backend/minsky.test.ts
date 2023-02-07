import {describe, expect, test} from '@jest/globals';
import {
  minsky,
  GodleyIcon,
  PlotWidget,
  VariableBase,
} from './index';
import * as fs from 'fs';
import * as JSON5 from 'json5';

var tmpDir;
beforeAll(()=>{
  tmpDir=fs.mkdtempSync("minsky-jest");
});
afterAll(()=>{
  fs.rmdirSync(tmpDir,{recursive: true});
});
           
beforeEach(()=>{
  minsky.clearAllMaps();
});

describe('Minsky load/save', ()=>{
  test('save empty',()=>{
    minsky.save(tmpDir+"/foo.mky");
  });
  test('load empty', ()=>{
    minsky.load(tmpDir+"/foo.mky");
  });
});

describe('Named Items',()=>{
  test('name item', ()=>{
    minsky.canvas.addOperation("time");
    minsky.canvas.getItemAt(0,0);
    minsky.nameCurrentItem("foo");
    console.log(JSON5.stringify(minsky.namedItems.elem("foo")));
    expect(minsky.namedItems.elem("foo").second.classType()).toBe("Operation:time");
    expect(minsky.namedItems.size()).toBe(1);
    minsky.namedItems.insert("fooBar",minsky.canvas.item.properties());
    expect(minsky.namedItems.size()).toBe(2);
    // TODO erase doesn't quite work
    minsky.namedItems.erase("foo");
    expect(minsky.namedItems.size()).toBe(1);
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
    expect((new VariableBase(minsky.canvas.item)).name()).toBe("foo");
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
    minsky.canvas.selection.clear();
    minsky.copy();
    expect(minsky.clipboardEmpty()).toBe(true);
    minsky.canvas.mouseDown(0,0);
    minsky.canvas.mouseUp(200,200);
    minsky.copy();
    expect(minsky.canvas.selection.empty()).toBe(false);
    expect(minsky.clipboardEmpty()).toBe(false);
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
    minsky.canvas.addOperation("time");
    expect(minsky.undo(0)).toBeGreaterThan(1);
    minsky.clearHistory();
    expect(minsky.undo(0)).toBe(1);
  });
  test('dimensional analysis',()=>{
    minsky.deleteAllUnits();
    expect(minsky.dimensionalAnalysis()).toStrictEqual({});
  });
});

describe('item casts',()=>{
  test('addGodley',()=>{
    minsky.canvas.addGodley();
    minsky.canvas.getItemAt(0,0);
    expect(minsky.canvas.item.classType()).toBe("GodleyIcon");
    let godley=new GodleyIcon(minsky.canvas.item);
    godley.table.resize(3,2);
    godley.setCell(0,1,"stock");
    godley.setCell(2,1,"flow");
    godley.update();
    minsky.canvas.copyAllFlowVars();
    minsky.canvas.copyAllStockVars();
  });
  test('addPlot',()=>{
    minsky.canvas.addPlot();
    minsky.canvas.getItemAt(0,0);
    expect(minsky.canvas.item.classType()).toBe("PlotWidget");
    let plot=new PlotWidget(minsky.canvas.item);
    expect(plot.plotType()).toBe("automatic");
    expect(plot.plotType("bar")).toBe("bar");
    minsky.canvas.copyItem();
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.model.items.elem(1).classType()).toBe("PlotWidget");
  });
});

describe('canvas',()=>{
  test('defaultRotation',()=>{
    expect(minsky.canvas.defaultRotation(90)).toBe(90);
    expect(minsky.canvas.defaultRotation()).toBe(90);
    expect(minsky.canvas.defaultRotation(0)).toBe(0);
    expect(minsky.canvas.defaultRotation()).toBe(0);
  });
  test('deleteItem',()=>{
    minsky.canvas.addPlot();
    expect(minsky.model.items.size()).toBe(1);
    minsky.canvas.getItemAt(0,0);
    minsky.canvas.deleteItem();
    expect(minsky.model.items.size()).toBe(0);
  });
  test('add/delete wire',()=>{
    minsky.canvas.addOperation("time");
    minsky.canvas.mouseUp(100,100); // insert op at 100,100
    minsky.canvas.addOperation("add");
    minsky.canvas.mouseUp(200,200); 
    minsky.canvas.getItemAt(100,100);
    let src=[minsky.canvas.item.portX(0),minsky.canvas.item.portY(0)];
    minsky.canvas.getItemAt(200,200);
    minsky.canvas.mouseDown(src[0],src[1]);
    minsky.canvas.mouseUp(minsky.canvas.item.portX(1),minsky.canvas.item.portY(1)); // should add a wire
    expect(minsky.model.wires.size()).toBe(1);
    minsky.canvas.getWireAt(150,150);
    expect(minsky.canvas.wire.properties()).not.toBe({});
    minsky.canvas.deleteWire();
    expect(minsky.model.wires.size()).toBe(0);
  });
});
