import { Component, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { ClassType, commandsMapping } from '@minsky/shared';

@Component({
  selector: 'minsky-edit-operation',
  templateUrl: './edit-operation.component.html',
  styleUrls: ['./edit-operation.component.scss'],
})
export class EditOperationComponent implements OnInit {
  form: FormGroup;

  classType: ClassType;
  itemType: string;

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
  }

  ngOnInit() {
    if (this.electronService.isElectron) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  private async updateFormValues() {
    const type = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_TYPE,
    })) as string;

    this.itemType = type;

    const rotation = await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_ROTATION,
    });
    this.rotation.setValue(rotation);

    const axis = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_AXIS,
    })) as string;
    this.axis.setValue(axis);

    const arg = (await this.electronService.sendMinskyCommandAndRender({
      render: false,
      command: commandsMapping.CANVAS_ITEM_ARG,
    })) as number;
    this.argument.setValue(arg);
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_ROTATION} ${this.rotation.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_AXIS} "${this.axis.value}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_ARG} ${this.argument.value}`,
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
