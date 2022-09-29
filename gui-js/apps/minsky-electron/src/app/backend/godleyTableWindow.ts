import {RenderNativeWindow} from './renderNativeWindow';

/** Exposes C++ GodleyTableWindow class into typescript. Please see C++ code documentation for more information. */
export class GodleyTableWindow extends RenderNativeWindow
{
  constructor(prefix: string) {super(prefix);}

  addFlow(y: number): void {this.callMethod("addFlow",y);}
  addStockVar(x: number): void {this.callMethod("addStockVar",x);}
  clickTypeZoomed(x: number,y: number): string {return this.callMethod("clickTypeZoomed",x,y);}
  copy(): void {this.callMethod("copy");}
  cut(): void {this.callMethod("cut");}
  colXZoomed(x: number): number {return this.callMethod("colXZoomed",x);}
  deleteFlow(y: number): void {this.callMethod("deleteFlow",y);}
  deleteStockVar(x: number): void {this.callMethod("deleteStockVar",x);}
  importStockVar(name: string,x: number): void {this.callMethod("importStockVar",name,x);}
  insertIdx(...i: number[]): number {return this.callMethod("insertIdx",...i);}
  matchingTableColumns(x: number): string[] {return this.callMethod("matchingTableColumns",x);}
  paste(): void  {this.callMethod("paste");}
  rowYZoomed(y: number): number{return this.callMethod("rowYZoomed",y);}
  selectedCol(...args: number[]): number {return this.callMethod("selectedCol",...args);}
  selectedIdx(...args: number[]): number {return this.callMethod("selectedIdx",...args);}
  selectedRow(...args: number[]): number {return this.callMethod("selectedRow",...args);}
  undo(changes: number): void {this.callMethod("undo", changes);}
  zoomFactor(...args: number[]): number {return this.callMethod("zoomFactor",...args);}
}
  
