import {CppClass} from './backend';

export class VariableInstanceList extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  gotoInstance(i: number): void {this.callMethod("gotoInstance");}
  names(): string[] {return this.callMethod("names");}
};
