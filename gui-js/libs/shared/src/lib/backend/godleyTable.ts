import {CppClass} from './backend';

/** Exposes C++ GodleyTable class into typescript. Please see C++ code documentation for more information. */
export class GodleyTable extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  exportToCSV(file: string): void {this.callMethod("exportToCSV",file);}
  exportToLaTeX(file: string): void {this.callMethod("exportToLaTeX",file);}
  getCell(row: number, col: number): string {return this.callMethod("getCell",row,col);}
  resize(rows: number, cols: number): void {this.callMethod("resize",rows,cols);}
  title(): string {return this.callMethod("title");}
}
