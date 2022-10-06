import {Item} from './item';

/** Exposes C++ Variable class into typescript. Please see
 * C++ code documentation for more information. */
export class Variable extends Item
{
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<Variable>x).prefix);
  }

  defined(): boolean {return this.callMethod("defined");}
  dims(): Array<number> {return this.callMethod("dims");}
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  local(): boolean {return this.callMethod("local");}
  name(...args: string[]): string {return this.callMethod("name",...args);}
  toggleLocal(): void {this.callMethod("toggleLocal");}
  toggleVarTabDisplay(): void {this.callMethod("toggleVarTabDisplay");}
  type(): string {return this.callMethod("type");}
  value(...args: number[]): number {return this.callMethod("value",...args);}

}
