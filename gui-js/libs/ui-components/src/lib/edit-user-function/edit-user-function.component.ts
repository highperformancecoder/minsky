import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, UserFunction } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'minsky-edit-user-function',
    templateUrl: './edit-user-function.component.html',
    styleUrls: ['./edit-user-function.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class EditUserFunctionComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  itemName: string;
  udf: UserFunction;
  
  public get name(): AbstractControl {
    return this.form.get('name');
  }
  public get rotation(): AbstractControl {
    return this.form.get('rotation');
  }
  public get expression(): AbstractControl {
    return this.form.get('expression');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.form = new FormGroup({
      name: new FormControl(''),
      rotation: new FormControl(0),
      expression: new FormControl(''),
    });
    this.route.queryParams.subscribe((params) => {
      this.classType = params.classType;
    });
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      this.udf=new UserFunction(this.electronService.minsky.canvas.item);
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    this.itemName = await this.udf.description();
    this.name.setValue(this.itemName);
    this.rotation.setValue(await this.udf.rotation());
    this.expression.setValue(await this.udf.expression());
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      this.udf.description(this.name.value);
      this.udf.rotation(this.rotation.value);
      this.udf.expression(this.expression.value);
      this.electronService.minsky.requestRedraw();
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
