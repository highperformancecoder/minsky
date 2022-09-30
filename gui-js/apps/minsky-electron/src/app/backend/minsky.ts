import {CppClass, Map, Container} from './backend';
import {Canvas} from './canvas';
import {Group} from './group';
import {Item} from './item';
import {ItemTab} from './itemTab';
import {PlotTab} from './plotTab';
import {VariableInstanceList} from './variableInstanceList';

/** Exposes C++ Minsky class into typescript. Please see C++ code documentation for more information. */
export class Minsky extends CppClass
{
  canvas: Canvas;
  logVarList: Container<string>;
  model: Group;
  namedItems: Map<string, Item>;
  plotTab: PlotTab;
  variableInstanceList: VariableInstanceList;
  variableTab: ItemTab;
  
  constructor(prefix: string) {
    super(prefix);
    this.canvas=new Canvas(prefix+"/canvas");
    this.logVarList=new Container<string>(prefix+"/logVarList");
    this.model=new Group(prefix+"/model");
    this.namedItems=new Map<string, Item>(prefix+"/namedItems",Item);
    this.plotTab=new PlotTab(prefix+"/plotTab");
    this.variableInstanceList=new VariableInstanceList(prefix+"/variableInstanceList");
    this.variableTab=new ItemTab(prefix+"/variableTab");
  }

  addIntegral(): void {this.callMethod("addIntegral");}
  availableOperations(): string[] {return this.callMethod("availableOperations");}
  classifyOp(op: string): string {return this.callMethod("classifyOp",op);}
  clearAllMaps(): void {this.callMethod("clearAllMaps");}
  clearHistory(): void {this.callMethod("clearHistory");}
  clipboardEmpty(): boolean {return this.callMethod("clipboardEmpty");}
  copy(): void {this.callMethod("copy");}
  cut(): void {this.callMethod("cut");}
  doPushHistory(...d: boolean[]): boolean {return this.callMethod("doPushHistory",...d);}
  edited(): boolean {return this.callMethod("edited");}
  nameCurrentItem(name: string) {this.callMethod("nameCurrentItem",name);}
  listAllInstances(): void {this.callMethod("listAllInstances");}
  load(file: string): void {this.callMethod("load",file);}
  multipleEquities(...args: boolean[]): boolean {return this.callMethod("multipleEquities",...args);}
  openGroupInCanvas(): void {this.callMethod("openGroupInCanvas");}
  openLogFile(file: string): void {this.callMethod("openLogFile",file);}
  openModelInCanvas(): void {this.callMethod("openModelInCanvas");}
  paste(): void {this.callMethod("paste");}
  popFlags(): void {this.callMethod("popFlags");}
  pushFlags(): void {this.callMethod("pushFlags");}
  reset(): void {this.callMethod("reset");}
  save(file: string): void {this.callMethod("save",file);}
  saveCanvasItemAsFile(file: string): void {this.callMethod("saveCanvasItemAsFile",file);}
  saveSelectionAsFile(name: string): void {this.callMethod("saveSelectionAsFile",name);}
  setAutoSaveFile(file: string): void {this.callMethod("setAutoSaveFile",file);}
  setGodleyDisplayValue(displayValues: boolean, displayStyle: string): void
  {this.callMethod("setGodleyDisplayValue",displayValues,displayStyle);}
  undo(changes: number): void {this.callMethod("undo",changes);}
};

/** global backend Minsky object */
export const minsky=new Minsky("/minsky");
