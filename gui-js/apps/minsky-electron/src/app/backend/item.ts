import {CppClass} from './backend';

/** Exposes C++ Item class into typescript. Please see
 * C++ code documentation for more information. */
export class Item extends CppClass
{
  constructor(prefix: string) {super(prefix);}

  classType(): string {return this.callMethod("classType");}
  description(): string {return this.callMethod("description");}
  flip(): void {this.callMethod("flip");}
  flipContents(): void {this.callMethod("flipContents");}
  id(): string {return this.callMethod("id");}
  numCases(): number {return this.callMethod("numCases");}
  readData(fileName: string): void {this.callMethod("readData", fileName);}
  setNumCases(numCases: number): void {this.callMethod("setNumCases",numCases);}
  value(): number {return this.callMethod("value");}
  x(...args: number[]): number {return this.callMethod("x",...args);}
  y(...args: number[]): number {return this.callMethod("y",...args);}
}

