import {Item} from './item';
import {RavelLockGroup} from './ravelLockGroup';

/** Exposes C++ Ravel class into typescript. Please see
 * C++ code documentation for more information. */
export class Ravel extends Item
{
  lockGroup: RavelLockGroup;
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<Ravel>x).prefix);
    this.lockGroup=new RavelLockGroup(this.prefix+"/lockGroup");
  }

  allSliceLabels(): string[] {return this.callMethod("allSliceLabels");}
  broadcastStateToLockGroup(): void {this.callMethod("broadcastStateToLockGroup");}
  dimensionType(...args: number[]): string {return this.callMethod("dimensionType",...args);}
  dimensionUnitsFormat (...args: number[]): string {return this.callMethod("dimensionUnitsFormat",...args);}
  editorMode(): boolean {return this.callMethod("editorMode");}
  handleDescription(h: number): string {return this.callMethod("handleDescription",h);}
  handleSetReduction(h: number, op: string): void {this.callMethod("handleSetReduction",h,op);}
  leaveLockGroup(): void {this.callMethod("leaveLockGroup");}
  locked(): boolean {return this.callMethod("locked");}
  nextReduction(op: string): void {this.callMethod("nextReduction");}
  pickedSliceLabels(): string[] {return this.callMethod("pickedSliceLabels");}
  pickSliceLabels(axis: number, pick: string[]): void {this.callMethod("pickSliceLabels",axis,pick);}
  selectedHandle(): number {return this.callMethod("selectedHandle");}
  setDimension(handle: number, type: string, units: string): void {this.callMethod("setDimension",handle,type,units);}
  setHandleDescription(handle: number, desc: string): void {this.callMethod("setHandleDescription",handle,desc);}
  setSortOrder(order: string): string {return this.callMethod("setSortOrder",order);}
  sortOrder(): string {return this.callMethod("sortOrder");}
  sortByValue(dir: string): void {this.callMethod("sortByValue", dir);}
  toggleDisplayFilterCaliper(): boolean {return this.callMethod("toggleDisplayFilterCaliper");}
  toggleEditorMode(): void {this.callMethod("toggleEditorMode");}
  toggleLocked(): void {this.callMethod("toggleLocked");}
}
