import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, OperationBase } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';

import { MatAutocompleteModule } from '@angular/material/autocomplete';

@Component({
    selector: 'minsky-edit-operation',
    templateUrl: './edit-operation.component.html',
    styleUrls: ['./edit-operation.component.scss'],
    standalone: true,
    imports: [
    FormsModule,
    ReactiveFormsModule,
    MatAutocompleteModule,
    MatOptionModule,
    MatButtonModule
],
})
export class EditOperationComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  itemType: string;
  op: OperationBase;
  axes: string[];
  
  public get axis(): AbstractControl {
    return this.form.get('axis');
  }
  public get argument(): AbstractControl {
    return this.form.get('argument');
  }
  public get rotation(): AbstractControl {
    return this.form.get('rotation');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.form = new FormGroup({
      axis: new FormControl(''),
      argument: new FormControl(null),
      rotation: new FormControl(0),
    });
    this.route.queryParams.subscribe((params) => {
      this.classType = params.classType;
    });
    this.op=new OperationBase(this.electronService.minsky.canvas.item);
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    this.itemType = await this.op.type();
    this.rotation.setValue(await this.op.rotation());
    this.axis.setValue(await this.op.axis());
    this.argument.setValue(await this.op.arg());
    this.axes=await this.op.dimensions();
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      this.op.rotation(this.rotation.value);
      this.op.axis(this.axis.value);
      this.op.arg(this.argument.value);
      this.electronService.minsky.requestRedraw();
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
