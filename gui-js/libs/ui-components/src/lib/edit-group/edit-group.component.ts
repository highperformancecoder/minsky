import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, Group } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'minsky-edit-group',
    templateUrl: './edit-group.component.html',
    styleUrls: ['./edit-group.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class EditGroupComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  group: Group;

  public get name(): AbstractControl {
    return this.form.get('name');
  }

  public get rotation(): AbstractControl {
    return this.form.get('rotation');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.form = new FormGroup({
      name: new FormControl(''),
      rotation: new FormControl(0),
    });
    this.route.queryParams.subscribe((params) => {
      this.classType = params.classType;
    });
    this.group=new Group(this.electronService.minsky.canvas.item);
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    this.name.setValue(await this.group.title());
    this.rotation.setValue(await this.group.rotation());
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      this.group.title(this.name.value);
      this.group.rotation(this.rotation.value);
      this.electronService.minsky.canvas.requestRedraw();
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
