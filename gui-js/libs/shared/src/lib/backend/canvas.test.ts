import {describe, expect, test} from '@jest/globals';
import {
  GodleyIcon,
  minsky,
} from './index';

beforeEach(()=>{
  minsky.clearAllMaps();
});

describe('smoke test',()=>{
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
    minsky.canvas.copyItem();
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.model.items.elem(1).classType()).toBe("PlotWidget");
  });
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
