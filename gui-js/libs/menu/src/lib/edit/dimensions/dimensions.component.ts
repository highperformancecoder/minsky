import { Component, OnInit } from '@angular/core';
import { FormArray, FormControl, FormGroup } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { commandsMapping, dateTimeFormats } from '@minsky/shared';

interface Second {
  type: string;
  units: string;
}

interface Dimension {
  dimension: string;
  type: Second;
  units: Second;
}
@Component({
  selector: 'minsky-dimensions',
  templateUrl: './dimensions.component.html',
  styleUrls: ['./dimensions.component.scss'],
})
export class DimensionsComponent implements OnInit {
  form: FormGroup;
  types = ['string', 'value', 'time'];

  timeFormatStrings = dateTimeFormats;

  public get dimensions(): FormArray {
    return this.form.get('dimensions') as FormArray;
  }

  constructor(private electronService: ElectronService) {
    this.form = new FormGroup({ dimensions: new FormArray([]) });
  }

  ngOnInit() {
    (async () => {
      if (this.electronService.isElectron) {
        const dimensions = (await this.electronService.sendMinskyCommandAndRender(
          {
            command: commandsMapping.DIMENSIONS,
          }
        )) as Record<string, Second>;

        for (const [key, args] of Object.entries(dimensions)) {
          this.dimensions.push(this.createDimension(key, args));
        }
      }
    })();
  }

  createDimension(key: string, args: Second) {
    return new FormGroup({
      dimension: new FormControl(key),
      type: new FormControl(args.type),
      units: new FormControl(args.units),
    });
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }

  getDimensions() {
    const dimensions = this.form.value.dimensions as Dimension[];

    return dimensions.reduce((acc, curr) => {
      acc[curr.dimension] = {
        type: curr.type,
        units: curr.units,
      };
      return acc;
    }, {});
  }

  async handleSubmit() {
    const dimensions = this.getDimensions();

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.DIMENSIONS} ${JSON.stringify(dimensions)}`,
    });

    this.closeWindow();
  }
}
