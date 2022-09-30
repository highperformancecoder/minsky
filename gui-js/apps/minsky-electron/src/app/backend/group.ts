import {Item} from './item';

export class Group extends Item
{
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<Group>x).prefix);
  }
  cBounds(): number[] {return this.callMethod("cBounds");}
  defaultExtension(): string {return this.callMethod("defaultExtension");}
  displayContents(): boolean {return this.callMethod("displayContents");}
  empty(): boolean {return this.callMethod("empty");}
  flipContents(): void {this.callMethod("flipContents");}
  makeSubroutine(): void {this.callMethod("makeSubroutine");}
  removeDisplayPlot(): void {this.callMethod("removeDisplayPlot");}
  setZoom(z: number): void {return this.callMethod("setZoom");}
};
