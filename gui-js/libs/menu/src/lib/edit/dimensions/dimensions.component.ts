import { Component, OnInit } from '@angular/core';
import { FormArray, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { dateTimeFormats } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';
import { MatAutocompleteModule } from '@angular/material/autocomplete';
import { NgFor, NgIf } from '@angular/common';

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
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        NgFor,
        NgIf,
        MatAutocompleteModule,
        MatOptionModule,
        MatButtonModule,
    ],
})
export class DimensionsComponent implements OnInit {
  form: FormGroup;
  types = ['string', 'value', 'time'];

  submittedDimensions: string;

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
        this.electronService.minsky.populateMissingDimensions();
        const dimensions = (await this.electronService.minsky.dimensions.$properties()) as Record<string, Second>;

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
    return this.dimensions.value.reduce((acc, curr) => {
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
    await this.electronService.minsky.dimensions.$properties(this.getDimensions());
    await this.electronService.minsky.imposeDimensions();
    this.electronService.minsky.reset();
    this.closeWindow();
  }
}
