import {ItemTab} from './itemTab';

/** Exposes C++ Variable class into typescript. Please see
 * C++ code documentation for more information. */
export class VariablePane extends ItemTab
{
  constructor(prefix: string) {super(prefix);}

  deselect(variableType: string): void {this.callMethod("deselect",variableType);}
  select(variableType: string): void {this.callMethod("select",variableType);}
  shift(...args: boolean[]): boolean {return this.callMethod("shift",...args);}
  updateWithHeight(height: number): void {this.callMethod("updateWithHeight",height);}
}
