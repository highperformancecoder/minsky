import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, IntOp} from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'minsky-edit-integral',
    templateUrl: './edit-integral.component.html',
    styleUrls: ['./edit-integral.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class EditIntegralComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  itemName: string;
  intOp: IntOp;

  public get name(): AbstractControl {
    return this.form.get('name');
  }

  public get initialValue(): AbstractControl {
    return this.form.get('initialValue');
  }
  public get units(): AbstractControl {
    return this.form.get('units');
  }
  public get rotation(): AbstractControl {
    return this.form.get('rotation');
  }
  public get relative(): AbstractControl {
    return this.form.get('relative');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.form = new FormGroup({
      name: new FormControl(''),
      initialValue: new FormControl(''),
      units: new FormControl(''),
      rotation: new FormControl(0),
      relative: new FormControl(false),
    });
    this.route.queryParams.subscribe((params) => {
      this.classType = params.classType;
    });
    this.intOp=new IntOp(electronService.minsky.canvas.item);
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    this.itemName = await this.intOp.description();
    this.name.setValue(this.itemName);
    this.rotation.setValue(await this.intOp.rotation());
    this.initialValue.setValue(await this.intOp.intVar.init());
    this.units.setValue(await this.intOp.intVar.units());
    this.relative.setValue(await this.intOp.intVar.sliderStepRel());
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      this.intOp.description(this.name.value);
      this.intOp.rotation(this.rotation.value);
      this.intOp.intVar.init(this.initialValue.value);
      this.intOp.intVar.setUnits(this.units.value);
      this.intOp.intVar.sliderStepRel(this.relative.value);
      this.electronService.minsky.requestRedraw();
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
