import {CppClass} from './backend';

export class GodleyTable extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  exportToCSV(file: string): void {this.callMethod("exportToCSV",file);}
  exportToLaTeX(file: string): void {this.callMethod("exportToLaTeX",file);}
  title(): string {return this.callMethod("title");}
}
