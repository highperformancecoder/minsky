import { Component, OnInit } from '@angular/core';
import { FormArray, FormControl, FormGroup } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { dateTimeFormats } from '@minsky/shared';

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
  originalDimensionNames: string[]=[];

  
  public get dimensions(): FormArray {
    return this.form.get('dimensions') as FormArray;
  }

  constructor(private electronService: ElectronService) {
    this.form = new FormGroup({ dimensions: new FormArray([]) });
  }

  ngOnInit() {
    (async () => {
      if (this.electronService.isElectron) {
        const dimensions = (await this.electronService.minsky.dimensions.properties()) as Record<string, Second>;

        for (const [key, args] of Object.entries(dimensions)) {
          this.originalDimensionNames.push(key);
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

  closeWindow() {this.electronService.closeWindow();}

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
    // handle renames
    let newDimensions=this.form.value.dimensions as Dimension[];
    for (let i=0; i<newDimensions.length; ++i)
      if (i<this.originalDimensionNames.length && newDimensions[i].dimension !== this.originalDimensionNames[i])
        this.electronService.minsky.renameDimension(this.originalDimensionNames[i],newDimensions[i].dimension); 
    this.electronService.minsky.dimensions.properties(this.getDimensions());
    this.electronService.minsky.imposeDimensions();
    this.electronService.minsky.reset();
    this.closeWindow();
  }
}
