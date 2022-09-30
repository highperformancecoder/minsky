import {Group} from './group';
import {Item} from './item';
import {RenderNativeWindow} from './renderNativeWindow';
import {Wire} from './wire';

/** Exposes C++ Canvas class into typescript. Please see C++ code documentation for more information. */
export class Canvas extends RenderNativeWindow
{
  item: Item;
  selection: Group
  wire: Wire;

  constructor(prefix: string) {
    super(prefix);
    this.selection=new Group(prefix+"/selection");
    this.item=new Item(prefix+"/item");
    this.wire=new Wire(prefix+"/wire");
  }
  
  addOperation(type: string): void {return this.callMethod("addOperation",type);}
  addGodley(): void {this.callMethod("addGodley");}
  addPlot(): void {this.callMethod("addPlot");}
  copyItem(): void {this.callMethod("copyItem");}
  copyAllFlowVars(): void {this.callMethod("copyAllFlowVars");}
  copyAllStockVars(): void {this.callMethod("copyAllStockVars");}
  defaultRotation(...args): number {return this.callMethod("defaultRotation");}
  defined(): boolean {return this.callMethod("defined");}
  deleteItem(): void {this.callMethod("deleteItem");}
  deleteWire(): void { this.callMethod("deleteWire");}
  findVariableDefinition(): boolean {return this.callMethod("findVariableDefinition");}
  flip(): void {this.callMethod("flip");}
  getItemAt(x: number, y: number): void {this.callMethod("getItemAt",x,y);}
  getWireAt(x: number, y: number): void {this.callMethod("getWireAt",x,y);}
  groupSelection(): void {this.callMethod("groupSelection");}
  itemIndicator(...i: boolean[]): boolean {return this.callMethod("itemIndicator",i);}
  lockRavelsInSelection():  void {this.callMethod("lockRavelsInSelection");}
  moveTo(x: number, y: number): number {return this.callMethod("moveTo",x,y);}
  pushDefiningVarsToTab(): void {this.callMethod("pushDefiningVarsToTab");}
  recentre(): void {this.callMethod("recentre");}
  removeItemFromItsGroup(): void {this.callMethod("removeItemFromItsGroup");}
  selectAllVariables(): void {this.callMethod("selectAllVariables");}
  selectVar(x: number, y: number): boolean {return this.callMethod("selectVar",x,y);}
  showPlotsOnTab(): void {this.callMethod("showPlotsOnTab");}
  showAllPlotsOnTab(): void {this.callMethod("showAllPlotsOnTab");}
  showDefiningVarsOnCanvas(): void {this.callMethod("showDefiningVarsOnCanvas");}
  ungroupItem(): void {this.callMethod("ungroupItem");}
  unlockRavelsInSelection(): void {this.callMethod("unlockRavelsInSelection");}
  zoomToDisplay(): void {this.callMethod("zoomToDisplay");}
};

