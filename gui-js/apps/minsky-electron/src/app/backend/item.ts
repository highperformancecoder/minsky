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
  buttonDisplay(): boolean {return this.callMethod("buttonDisplay");}
  classType(): string {return this.callMethod("classType");}
  displayContents(): boolean {return this.callMethod("displayContents");}
  defined(): boolean {return this.callMethod("defined");}
  description(): string {return this.callMethod("description");}
  destroyFrame(): void {this.callMethod("destroyFrame");}
  dims(): Array<number> {return this.callMethod("dims");}
  editorMode(): boolean {return this.callMethod("editorMode");}
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  flip(): void {this.callMethod("flip");}
  flipContents(): void {this.callMethod("flipContents");}
  id(): string {return this.callMethod("id");}
  leaveLockGroup(): void {this.callMethod("leaveLockGroup");}
  local(): boolean {return this.callMethod("local");}
  locked(): boolean {return this.callMethod("locked");}
  lockGroup(): any {return this.callMethod("lockGroup");}
  makeDisplayPlot(): void {this.callMethod("makeDisplayPlot");}
  makeSubroutine(): void {this.callMethod("makeSubroutine");}
  name(...args: string[]): string {return this.callMethod("name",...args);}
  numCases(): number {return this.callMethod("numCases");}
  plotTabDisplay(...args: boolean[]): boolean {return this.callMethod("plotTabDisplay",...args);}
  portValues(): string {return this.callMethod("portValues");}
  readData(fileName: string): void {this.callMethod("readData", fileName);}
  removeDisplayPlot(): void {this.callMethod("removeDisplayPlot");}
  setNumCases(numCases: number): void {this.callMethod("setNumCases",numCases);}
  toggleButtons(): void {this.callMethod("toggleButtons");}
  toggleCoupled(): boolean {return this.callMethod("toggleCoupled");}
  toggleEditorMode(): void {this.callMethod("toggleEditorMode");}
  toggleLocal(): void {this.callMethod("toggleLocal");}
  toggleLocked(): void {this.callMethod("toggleLocked");}
  togglePlotTabDisplay(): void {this.callMethod("togglePlotTabDisplay");}
  toggleVariableDisplay(): void {this.callMethod("toggleVariableDisplay");}
  toggleVarTabDisplay(): void {this.callMethod("toggleVarTabDisplay");}
  type(): string {return this.callMethod("type");}
  value(...args: number[]): number {return this.callMethod("value",...args);}
  variableDisplay(...args: boolean[]): boolean {return this.callMethod("variableDisplay",...args);}
  varTabDisplay(...args: boolean[]): boolean {return this.callMethod("varTabDisplay",...args);}
  x(...args: number[]): number {return this.callMethod("x",...args);}
  y(...args: number[]): number {return this.callMethod("y",...args);}
}

