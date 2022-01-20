import { Component, OnInit } from '@angular/core';
import { FormArray, FormControl, FormGroup } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';

enum DashStyles {
  SOLID = 'solid',
  DASH = 'dash',
  DOT = 'dot',
  DASH_DOT = 'dashDot',
}

interface Colour {
  r: number;
  g: number;
  b: number;
  a: number;
}

interface Palette {
  colour: string | Colour;
  dashStyle: DashStyles;
  width: number;
}

@Component({
  selector: 'minsky-pen-styles',
  templateUrl: './pen-styles.component.html',
  styleUrls: ['./pen-styles.component.scss'],
})
export class PenStylesComponent implements OnInit {
  form: FormGroup;
  palette: Palette[];

  dashStyles = ['solid', 'dash', 'dot', 'dashDot'];
  rgbMaxValue = 255;

  public get pens(): FormArray {
    return this.form.get('pens') as FormArray;
  }

  constructor(private electronService: ElectronService) {
    this.form = new FormGroup({
      pens: new FormArray([]),
    });
  }

  ngOnInit() {
    (async () => {
      if (this.electronService.isElectron) {
        this.palette = (await this.electronService.sendMinskyCommandAndRender({
          command: commandsMapping.CANVAS_PLOT_PALETTE,
        })) as Palette[];
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

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_PLOT_PALETTE} ${JSON.stringify(
        palette
      )}`,
    });

    this.closeWindow();
  }

  addPen() {
    const p: Palette = {
      colour: { r: 100, g: 100, b: 100, a: 1 },
      dashStyle: DashStyles.DASH,
      width: 1,
    };
    this.pens.push(this.createPen(p));
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
