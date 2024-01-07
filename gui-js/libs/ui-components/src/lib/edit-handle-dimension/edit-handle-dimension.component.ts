import { Component, OnInit, DoCheck } from '@angular/core';
import { FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { dateTimeFormats } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';
import { NgIf, NgFor } from '@angular/common';
import { MatAutocompleteModule } from '@angular/material/autocomplete';

@Component({
    selector: 'minsky-edit-handle-dimension',
    templateUrl: './edit-handle-dimension.component.html',
    styleUrls: ['./edit-handle-dimension.component.scss', '../generic-form.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatAutocompleteModule,
        NgIf,
        NgFor,
        MatOptionModule,
        MatButtonModule,
    ],
})
export class EditHandleDimensionComponent implements OnInit, DoCheck {
  editDimensionForm: FormGroup;
  timeFormatStrings = dateTimeFormats;

  command: string;
  handleIndex: number;

  tooltips = {
    string: 'unit is not applicable to string type',
    value:'enter a unit string, eg m/s',
    time: 'enter a strftime format string, eg %Y-%m-%d %H:%M:%S, or %Y-Q%Q'
  }

  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.command=params['command'];
      this.handleIndex = +params['handleIndex'];

      this.editDimensionForm = new FormGroup({
        type: new FormControl(params['type']),
        units: new FormControl(params['units']),
      });
    });
  }

  ngDoCheck() {
    const typeControl = this.editDimensionForm.get('type');
    const unitsControl = this.editDimensionForm.get('units');

    if(typeControl.value === 'string') {
      if(unitsControl.enabled) {
        unitsControl.disable();
        unitsControl.setValue('');
      }
    } else {
      if(!unitsControl.enabled) {
        unitsControl.enable();
      }
    }
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.saveHandleDimension({
        command: this.command,
        handleIndex: this.handleIndex,
        type: this.editDimensionForm.get('type').value,
        units: this.editDimensionForm.get('units').value
      });
    }
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
