import { Component, OnDestroy } from '@angular/core';
import { Subject } from 'rxjs';
import { FormArray, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { events } from '@minsky/shared';
import { takeUntil } from 'rxjs';
import { NgFor } from '@angular/common';

@Component({
    selector: 'minsky-log-simulation',
    templateUrl: './log-simulation.component.html',
    styleUrls: ['./log-simulation.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        NgFor,
    ],
})
export class LogSimulationComponent implements OnDestroy {
  form = new FormGroup({
    all: new FormControl(false),
    keys: new FormArray([]),
  });

  destroy$ = new Subject<{}>();

  public get keysControl(): FormArray {
    return this.form.get('keys') as FormArray;
  }

  public get allControl(): FormControl {
    return this.form.get('all') as FormControl;
  }

  constructor(private electronService: ElectronService) {
    (async () => {
      if (this.electronService.isElectron) {
        const variableValues = await this.electronService.minsky.variableValues.keys();

        variableValues
          .filter((key) => !key.includes('constant'))
          .forEach((key) => {
            this.keysControl.push(
              new FormGroup({
                key: new FormControl(key.split(':')[1]),
                keyOriginal: new FormControl(key),
                isSelected: new FormControl(false),
              })
            );
          });
      }
    })();

    this.allControl.valueChanges.pipe(takeUntil(this.destroy$)).subscribe((v) => {
      if (v) {
        this.keysControl.controls.forEach((control) => {
          control.get('isSelected').setValue(true);
        });
      } else {
        this.keysControl.controls.forEach((control) => {
          control.get('isSelected').setValue(false);
        });
      }
    });
  }

  closeWindow() {this.electronService.closeWindow();}

  handleSubmit() {
    const selectedItems = [];

    this.keysControl.controls.forEach((control) => {
      const isSelected = control.get('isSelected').value;
      if (isSelected) {
        selectedItems.push(control.get('keyOriginal').value);
      }
    });

    this.electronService.send(events.LOG_SIMULATION, selectedItems);

    this.closeWindow();
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
