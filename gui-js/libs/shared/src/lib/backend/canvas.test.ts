import {describe, expect, test} from '@jest/globals';
import {
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
  });
  test('addPlot',()=>{
    minsky.canvas.addPlot();
    minsky.canvas.getItemAt(0,0);
    expect(minsky.canvas.item.classType()).toBe("PlotWidget");
    minsky.canvas.copyItem();
    expect(minsky.model.items.size()).toBe(2);
    expect(minsky.model.items.elem(1).classType()).toBe("PlotWidget");
  });
});
  
