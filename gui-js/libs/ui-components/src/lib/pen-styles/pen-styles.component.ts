import { Component, OnInit } from '@angular/core';
import { FormArray, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { PlotWidget } from '@minsky/shared';
import { ColorPickerComponent, ColorPickerDirective } from 'ngx-color-picker';
import { LatexDirective } from '../directives/latex.directive';


interface Colour {
  r: number;
  g: number;
  b: number;
  a: number;
}

interface Palette {
  colour: string | Colour;
  dashStyle: string;
  plotType: string;
  width: number;
}

@Component({
    selector: 'minsky-pen-styles',
    templateUrl: './pen-styles.component.html',
    styleUrls: ['./pen-styles.component.scss'],
    standalone: true,
    imports: [
    FormsModule,
    ReactiveFormsModule,
    LatexDirective,
    ColorPickerComponent,
    ColorPickerDirective
],
})
export class PenStylesComponent implements OnInit {
  form: FormGroup;
  palette=[];
  penLabels=[];
  plot: PlotWidget;

  dashStyles = ['solid', 'dash', 'dot', 'dashDot'];
  plotTypes = ['automatic', 'line', 'bar', 'scatter', 'line_scatter'];
  rgbMaxValue = 255;

  public get pens(): FormArray {
    return this.form.get('pens') as FormArray;
  }

  constructor(private electronService: ElectronService) {
    this.form = new FormGroup({
      pens: new FormArray([]),
    });
    this.plot=new PlotWidget(electronService.minsky.canvas.item);
  }

  ngOnInit() {
    (async () => {
      if (this.electronService.isElectron) {
        this.palette = await this.plot.palette.properties();
        this.penLabels=await this.plot.penLabels.properties();
      }

      this.palette.forEach((p) => {
        this.pens.push(this.createPen(p));
      });
    })();
  }

  private createPen(p: Palette) {
    return new FormGroup({
      colour: new FormControl(this.getColor(p?.colour as Colour)),
      width: new FormControl(p?.width),
      dashStyle: new FormControl(p?.dashStyle),
      plotType: new FormControl(p?.plotType),
    });
  }

  public getColor(colour: Colour) {
    const { r, g, b, a } = colour;
    const rgbaString = `rgba(${r * this.rgbMaxValue},${g * this.rgbMaxValue},${
      b * this.rgbMaxValue
    },${a})`;

    return rgbaString;
  }

  convertColorStringToRgba(colorString: string) {
    const [r, g, b, a] = colorString.slice(0, -1).split('(')[1].split(',');

    return {
      r: Number(r) / this.rgbMaxValue,
      g: Number(g) / this.rgbMaxValue,
      b: Number(b) / this.rgbMaxValue,
      a: Number(a),
    };
  }

  async handleSubmit() {
    const palette = (this.form.value.pens as Palette[]).map((p) => {
      return {
        ...p,
        colour: this.convertColorStringToRgba(p.colour as string),
      };
    });

    this.plot.palette.properties(palette);
    this.electronService.minsky.requestRedraw();
    this.closeWindow();
  }

  addPen() {
    const p: Palette = {
      colour: { r: 100, g: 100, b: 100, a: 1 },
      dashStyle: 'dash',
      plotType: 'automatic',
      width: 1,
    };
    this.pens.push(this.createPen(p));
  }

  closeWindow() {this.electronService.closeWindow();}
}
