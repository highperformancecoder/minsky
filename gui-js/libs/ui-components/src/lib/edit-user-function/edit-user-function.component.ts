import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, commandsMapping, replaceBackSlash } from '@minsky/shared';

@Component({
  selector: 'minsky-edit-user-function',
  templateUrl: './edit-user-function.component.html',
  styleUrls: ['./edit-user-function.component.scss'],
})
export class EditUserFunctionComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  itemName: string;

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
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    const name = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_DESCRIPTION,
    })) as string;

    this.itemName = name;

    this.name.setValue(name);

    const rotation = await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_ROTATION,
    });
    this.rotation.setValue(rotation);

    const expression = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_EXPRESSION,
    })) as string;
    this.expression.setValue(expression);
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${
          commandsMapping.CANVAS_ITEM_DESCRIPTION
        } "${replaceBackSlash(this.name.value)}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_ROTATION} ${this.rotation.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_EXPRESSION} "${replaceBackSlash(
          this.expression.value
        )}"`,
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
