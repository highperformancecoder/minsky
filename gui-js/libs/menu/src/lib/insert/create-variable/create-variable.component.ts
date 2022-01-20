import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormControl,
  FormGroup,
  Validators,
} from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import {
  CommunicationService,
  ElectronService,
  WindowUtilityService,
} from '@minsky/core';
import { commandsMapping, replaceBackSlash } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-create-variable',
  templateUrl: './create-variable.component.html',
  styleUrls: ['./create-variable.component.scss'],
})
export class CreateVariableComponent implements OnInit, OnDestroy {
  variableType: string;
  _name: string;
  _value: string;

  isEditMode = false;

  form: FormGroup;

  public get variableName(): AbstractControl {
    return this.form.get('variableName');
  }

  public get value(): AbstractControl {
    return this.form.get('value');
  }

  public get units(): AbstractControl {
    return this.form.get('units');
  }

  public get rotation(): AbstractControl {
    return this.form.get('rotation');
  }

  public get type(): AbstractControl {
    return this.form.get('type');
  }

  public get shortDescription(): AbstractControl {
    return this.form.get('shortDescription');
  }

  public get detailedDescription(): AbstractControl {
    return this.form.get('detailedDescription');
  }

  public get sliderBoundsMax(): AbstractControl {
    return this.form.get('sliderBoundsMax');
  }

  public get sliderBoundsMin(): AbstractControl {
    return this.form.get('sliderBoundsMin');
  }

  public get sliderStepSize(): AbstractControl {
    return this.form.get('sliderStepSize');
  }

  public get sliderStepRel(): AbstractControl {
    return this.form.get('sliderStepRel');
  }

  constructor(
    public commService: CommunicationService,
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.subscribe((params) => {
      this.variableType = params.type;
      this._name = params?.name || '';
      this.isEditMode = params?.isEditMode || false;
      this._value = params?.value || '';
    });

    this.form = new FormGroup({
      variableName: new FormControl(this._name, Validators.required),
      type: new FormControl(this.variableType, Validators.required),
      value: new FormControl(this._value),
      units: new FormControl(''),
      rotation: new FormControl(0),
      shortDescription: new FormControl(''),
      detailedDescription: new FormControl(''),
      sliderBoundsMax: new FormControl(null),
      sliderBoundsMin: new FormControl(null),
      sliderStepSize: new FormControl(null),
      sliderStepRel: new FormControl(false),
    });

    this.type.value === 'constant'
      ? this.variableName.disable()
      : this.variableName.enable();

    this.type.valueChanges.subscribe((type) => {
      type === 'constant'
        ? this.variableName.disable()
        : this.variableName.enable();
    });
  }

  ngOnInit() {
    if (this.isEditMode) {
      (async () => {
        await this.updateFormValues();
      })();
    }
  }

  async updateFormValues() {
    const init = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_INIT,
    });

    const units = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_UNITS_STR,
    });

    const rotation = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_ROTATION,
    });
    const tooltip = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_TOOLTIP,
    });
    const detailedText = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_DETAILED_TEXT,
    });
    const sliderMax = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_SLIDER_MAX,
    });
    const sliderMin = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_SLIDER_MIN,
    });
    const sliderStep = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.CANVAS_ITEM_SLIDER_STEP,
    });
    const sliderStepRel = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.CANVAS_ITEM_SLIDER_STEP_REL,
      }
    );

    this.value.setValue(init);
    this.units.setValue(units);
    this.rotation.setValue(rotation);
    this.shortDescription.setValue(tooltip);
    this.detailedDescription.setValue(detailedText);
    this.sliderBoundsMax.setValue(sliderMax);
    this.sliderBoundsMin.setValue(sliderMin);
    this.sliderStepSize.setValue(sliderStep);
    this.sliderStepRel.setValue(sliderStepRel);
  }

  async handleSubmit() {
    if (this.isEditMode) {
      await this.editVariable();
      return;
    }
    await this.createVariable();
  }

  async editVariable() {
    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.RENAME_ITEM} "${replaceBackSlash(
        this.variableName.value
      )}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_RETYPE} "${this.type.value}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_SET_UNITS} "${replaceBackSlash(
        this.units.value || ''
      )}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_INIT} "${replaceBackSlash(
        this.value.value
      )}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_ROTATION} ${
        this.rotation.value || 0
      }`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_TOOLTIP} "${replaceBackSlash(
        this.shortDescription.value
      )}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${
        commandsMapping.CANVAS_ITEM_DETAILED_TEXT
      } "${replaceBackSlash(this.detailedDescription.value)}"`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_SLIDER_MAX} ${
        this.sliderBoundsMax.value || 0
      }`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_SLIDER_MIN} ${
        this.sliderBoundsMin.value || 0
      }`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_SLIDER_STEP} ${
        this.sliderStepSize.value || 0
      }`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.CANVAS_ITEM_SLIDER_STEP_REL} ${this.sliderStepRel.value}`,
    });

    this.closeWindow();
  }

  async createVariable() {
    await this.commService.createVariable({
      variableName: replaceBackSlash(this.variableName.value),
      type: this.type.value,
      units: replaceBackSlash(this.units.value || ''),
      value: replaceBackSlash(this.value.value),
      rotation: this.rotation.value || 0,
      shortDescription: replaceBackSlash(this.shortDescription.value),
      detailedDescription: replaceBackSlash(this.detailedDescription.value),
      sliderStepSize: this.sliderStepSize.value || 0,
      sliderBoundsMax: this.sliderBoundsMax.value || 0,
      sliderBoundsMin: this.sliderBoundsMin.value || 0,
    });
    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.windowUtilityService.closeCurrentWindowIfNotMain();
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
