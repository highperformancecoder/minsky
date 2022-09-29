import {CppClass} from './backend';

export class Wire extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  straighten(): void {this.callMethod("straighten");}
  visible(): boolean {return this.callMethod("visible");}
}

