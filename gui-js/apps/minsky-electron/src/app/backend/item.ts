import {GodleyTable} from './godleyTable';
import {RavelLockGroup} from './ravelLockGroup';
import {RenderNativeWindow} from './renderNativeWindow';

/** A union class for all Item types. Please see C++ code documentation for more information.  */
export class Item extends RenderNativeWindow
{
  lockGroup: RavelLockGroup;
  popup: RenderNativeWindow;
  table:GodleyTable;

  constructor(prefix: string) {
    super(prefix);
    this.lockGroup=new RavelLockGroup(prefix+"/lockGroup");
    this.popup=new RenderNativeWindow(prefix+"/popup");
    this.table=new GodleyTable(prefix+"/table");
  }

  allSliceLabels(): string[] {return this.callMethod("allSliceLabels");}
  adjustPopupWidgets(): void {this.callMethod("adjustPopupWidgets");}
  broadcastStateToLockGroup(): void {this.callMethod("broadcastStateToLockGroup");}
  buttonDisplay(): boolean {return this.callMethod("buttonDisplay");}
  classType(): string {return this.callMethod("classType");}
  displayContents(): boolean {return this.callMethod("displayContents");}
  defined(): boolean {return this.callMethod("defined");}
  description(): string {return this.callMethod("description");}
  destroyFrame(): void {this.callMethod("destroyFrame");}
  dimensionType(...args: number[]): string {return this.callMethod("dimensionType",...args);}
  dimensionUnitsFormat (...args: number[]): string {return this.callMethod("dimensionUnitsFormat",...args);}
  dims(): Array<number> {return this.callMethod("dims");}
  editorMode(): boolean {return this.callMethod("editorMode");}
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  flip(): void {this.callMethod("flip");}
  flipContents(): void {this.callMethod("flipContents");}
  handleDescription(h: number): string {return this.callMethod("handleDescription",h);}
  handleSetReduction(h: number, op: string): void {this.callMethod("handleSetReduction",h,op);}
  id(): string {return this.callMethod("id");}
  leaveLockGroup(): void {this.callMethod("leaveLockGroup");}
  local(): boolean {return this.callMethod("local");}
  locked(): boolean {return this.callMethod("locked");}
  makeDisplayPlot(): void {this.callMethod("makeDisplayPlot");}
  makeSubroutine(): void {this.callMethod("makeSubroutine");}
  name(...args: string[]): string {return this.callMethod("name",...args);}
  nextReduction(op: string): void {this.callMethod("nextReduction");}
  numCases(): number {return this.callMethod("numCases");}
  pickedSliceLabels(): string[] {return this.callMethod("pickedSliceLabels");}
  pickSliceLabels(axis: number, pick: string[]): void {this.callMethod("pickSliceLabels",axis,pick);}
  plotTabDisplay(...args: boolean[]): boolean {return this.callMethod("plotTabDisplay",...args);}
  portValues(): string {return this.callMethod("portValues");}
  readData(fileName: string): void {this.callMethod("readData", fileName);}
  removeDisplayPlot(): void {this.callMethod("removeDisplayPlot");}
  selectedHandle(): number {return this.callMethod("selectedHandle");}
  setDimension(handle: number, type: string, units: string): void {this.callMethod("setDimension",handle,type,units);}
  setHandleDescription(handle: number, desc: string): void {this.callMethod("setHandleDescription",handle,desc);}
  setNumCases(numCases: number): void {this.callMethod("setNumCases",numCases);}
  setSortOrder(order: string): string {return this.callMethod("setSortOrder",order);}
  sortOrder(): string {return this.callMethod("sortOrder");}
  sortByValue(dir: string): void {this.callMethod("sortByValue", dir);}
  toggleButtons(): void {this.callMethod("toggleButtons");}
  toggleCoupled(): boolean {return this.callMethod("toggleCoupled");}
  toggleDisplayFilterCaliper(): boolean {return this.callMethod("toggleDisplayFilterCaliper");}
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

