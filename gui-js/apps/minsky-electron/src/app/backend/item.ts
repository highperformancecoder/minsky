import {GodleyTable} from './godleyTable';
import {RenderNativeWindow} from './renderNativeWindow';

/** A union class for all Item types */
export class Item extends RenderNativeWindow
{
  popup: RenderNativeWindow;
  table:GodleyTable;

  constructor(prefix: string) {
    super(prefix);
    this.popup=new RenderNativeWindow(prefix+"/popup");
    this.table=new GodleyTable(prefix+"/table");
  }

  adjustPopupWidgets(): void {this.callMethod("adjustPopupWidgets");}
  classType(): string {return this.callMethod("classType");}
  displayContents(): boolean {return this.callMethod("displayContents");}
  defined(): boolean {return this.callMethod("defined");}
  description(): string {return this.callMethod("description");}
  destroyFrame(): void {this.callMethod("destroyFrame");}
  dims(): Array<number> {return this.callMethod("dims");}
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  flip(): void {this.callMethod("flip");}
  id(): string {return this.callMethod("id");}
  local(): boolean {return this.callMethod("local");}
  locked(): boolean {return this.callMethod("locked");}
  lockGroup(): any {return this.callMethod("lockGroup");}
  name(...args: string[]): string {return this.callMethod("name",...args);}
  numCases(): number {return this.callMethod("numCases");}
  setNumCases(numCases: number): void {this.callMethod("setNumCases",numCases);}
  type(): string {return this.callMethod("type");}
  value(...args: number[]): number {return this.callMethod("value",...args);}
  varTabDisplay(...args: boolean[]): boolean {return this.callMethod("varTabDisplay",...args);}
  x(...args: number[]): number {return this.callMethod("x",...args);}
  y(...args: number[]): number {return this.callMethod("y",...args);}
}

