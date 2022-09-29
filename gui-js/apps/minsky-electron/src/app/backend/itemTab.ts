import {Item} from './item';
import {RenderNativeWindow} from './renderNativeWindow';

export class ItemTab extends RenderNativeWindow
{
  item: Item;
  constructor(prefix: string) {
    super(prefix);
    this.item=new Item(prefix+"/item");
  }
  clickType(x: number, y: number): string {return this.callMethod("clickType",x,y);}
  colX(x: number): number {return this.callMethod("colX",x);}
  getItemAt(x: number, y: number): void {this.callMethod("getItemAt",x,y);}
  getVarName(row: number): string {return this.callMethod("getVarName",row);}
  rowY(y: number): number {return this.callMethod("rowY",y);}
  toggleVarDisplay(row: number): void {this.callMethod("toggleVarDisplay",row);}
}
