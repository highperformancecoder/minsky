import {Item} from './item';
import {RenderNativeWindow} from './renderNativeWindow';
/** Exposes C++ PlotWidget class into typescript. Please see
 * C++ code documentation for more information. */
export class PlotWidget extends Item 
{
  popup: RenderNativeWindow;
  constructor(x: string|Item) {
    super(typeof x==="string"? x: (<PlotWidget>x).prefix);
    // PlotWidget inherits directly from the RenderNativeWindow class
    this.popup=new RenderNativeWindow(this.prefix);
  }
  
  exportAsCSV(name: string): void {this.callMethod("exportAsCSV",name);}
  makeDisplayPlot(): void {this.callMethod("makeDisplayPlot");}
  plotTabDisplay(...args: boolean[]): boolean {return this.callMethod("plotTabDisplay",...args);}
  togglePlotTabDisplay(): void {this.callMethod("togglePlotTabDisplay");}

}

