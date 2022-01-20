import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, commandsMapping, replaceBackSlash } from '@minsky/shared';

@Component({
  selector: 'minsky-edit-group',
  templateUrl: './edit-group.component.html',
  styleUrls: ['./edit-group.component.scss'],
})
export class EditGroupComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;

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
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    const title = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_TITLE,
    })) as string;

    this.name.setValue(title);

    const rotation = await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_ROTATION,
    });
    this.rotation.setValue(rotation);
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_TITLE} "${replaceBackSlash(
          this.name.value
        )}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_ROTATION} ${this.rotation.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: commandsMapping.REQUEST_REDRAW_SUBCOMMAND,
      });
    }

    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
