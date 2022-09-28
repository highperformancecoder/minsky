import {Item} from './item';

export class Group extends Item
{
  constructor(prefix: string) {super(prefix);}
  defaultExtension(): string {return this.callMethod("defaultExtension");}
  setZoom(z: number): void {return this.callMethod("setZoom");}
};
