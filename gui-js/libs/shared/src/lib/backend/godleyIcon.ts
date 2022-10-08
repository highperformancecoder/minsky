import {GodleyTable} from './godleyTable';
import {GodleyTableWindow} from './godleyTableWindow';
import {Item} from './item';

/** Exposes C++ GodleyIcon class into typescript. Please see
 * C++ code documentation for more information. */
export class GodleyIcon extends Item
{
  popup: GodleyTableWindow;
  table:GodleyTable;

  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<GodleyIcon>x).prefix);
    this.popup=new GodleyTableWindow(this.prefix+"/popup");
    this.table=new GodleyTable(this.prefix+"/table");
  }

  adjustPopupWidgets(): void {this.callMethod("adjustPopupWidgets");}
  buttonDisplay(): boolean {return this.callMethod("buttonDisplay");}
  editorMode(): boolean {return this.callMethod("editorMode");}
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  setCell(row: number, col: number, value: string): void {this.callMethod("setCell",row,col,value);}
  toggleButtons(): void {this.callMethod("toggleButtons");}
  toggleEditorMode(): void {this.callMethod("toggleEditorMode");}
  toggleVariableDisplay(): void {this.callMethod("toggleVariableDisplay");}
  update(): void {this.callMethod("update");}
  variableDisplay(...args: boolean[]): boolean {return this.callMethod("variableDisplay",...args);}

}
