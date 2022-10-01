import {Sequence} from './backend';
import {Item} from './item';
import {Wire} from './wire';

export class Group extends Item
{
  items: Sequence<Item>;
  wires: Sequence<Wire>;
  groups: Sequence<Group>;
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<Group>x).prefix);
    this.items=new Sequence<Item>(this.prefix+"/items",Item);
    this.wires=new Sequence<Wire>(this.prefix+"/wires",Wire);
    this.groups=new Sequence<Group>(this.prefix+"/groups",Group);
  }
  bookmarkList(): string[] {return this.callMethod("bookmarkList");}
  cBounds(): number[] {return this.callMethod("cBounds");}
  defaultExtension(): string {return this.callMethod("defaultExtension");}
  deleteBookmark(i: number): void {this.callMethod("deleteBookmark",i);}
  displayContents(): boolean {return this.callMethod("displayContents");}
  empty(): boolean {return this.callMethod("empty");}
  flipContents(): void {this.callMethod("flipContents");}
  gotoBookmark(i: number): void {this.callMethod("gotoBookmark",i);}
  makeSubroutine(): void {this.callMethod("makeSubroutine");}
  removeDisplayPlot(): void {this.callMethod("removeDisplayPlot");}
  setZoom(z: number): void {return this.callMethod("setZoom");}
};
