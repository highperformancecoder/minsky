import {Item} from './item';
import {RenderNativeWindow} from './renderNativeWindow';
import {Wire} from './wire';

export class Canvas extends RenderNativeWindow
{
  item: Item;
  wire: Wire;
  constructor(prefix: string) {
    super(prefix);
    this.item=new Item(prefix+"/item");
    this.wire=new Wire(prefix+"/wire");
  }
  addOperation(type: string): void {return this.callMethod("addOperation",type);}
  addGodley(): void {return this.callMethod("addGodley");}
  addPlot(): void {return this.callMethod("addPlot");}
  defaultRotation(...args): number {return this.callMethod("defaultRotation");}
  defined(): boolean {return this.callMethod("defined");}
  deleteItem(): void {return this.callMethod("deleteItem");}
  findVariableDefinition(): boolean {return this.callMethod("findVariableDefinition");}
  flip(): void {return this.callMethod("flip");}
  getItemAt(x: number, y: number): void {return this.callMethod("getItemAt",x,y);}
  getWireAt(x: number, y: number): void {return this.callMethod("getWireAt",x,y);}
  itemIndicator(...i: boolean[]): boolean {return this.callMethod("itemIndicator",i);}
  moveTo(x: number, y: number): number {return this.callMethod("moveTo",x,y);}
  recentre(): void {this.callMethod("recentre");}
  selectVar(x: number, y: number): boolean {return this.callMethod("selectVar",x,y);}
};

