import {Item} from './item';

/** Exposes C++ Operation class into typescript. Please see
 * C++ code documentation for more information. */
export class Operation extends Item
{
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<Operation>x).prefix);
  }

  description(...d: string[]): string {return this.callMethod("description",...d);}
  portValues(): string {return this.callMethod("portValues");}
  toggleCoupled(): boolean {return this.callMethod("toggleCoupled");}
  type(): string {return this.callMethod("type");}
}
