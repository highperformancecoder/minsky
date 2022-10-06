import {ItemTab} from "./itemTab";

export class PlotTab extends ItemTab
{
  constructor(prefix: string) {super(prefix);}
  togglePlotDisplay(): void {this.callMethod("togglePlotDisplay");}
}
