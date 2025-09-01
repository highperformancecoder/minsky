import { Component, OnDestroy, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, Validators, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { Subject, takeUntil } from 'rxjs'; 
import { ActivatedRoute } from '@angular/router';
import {
  CommunicationService,
  ElectronService,
  WindowUtilityService,
} from '@minsky/core';
import { VariableBase } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';
import { MatAutocompleteModule } from '@angular/material/autocomplete';


@Component({
    selector: 'minsky-create-variable',
    templateUrl: './create-variable.component.html',
    styleUrls: ['./create-variable.component.scss'],
    standalone: true,
    imports: [
    FormsModule,
    ReactiveFormsModule,
    MatButtonModule,
    MatAutocompleteModule,
    MatOptionModule
],
})
export class CreateVariableComponent implements OnInit, OnDestroy {
  variableType: string;
  _name: string;
  _value: string;
  _local: boolean;
  isEditMode = false;

  destroy$ = new Subject<{}>();

  form: FormGroup;

  public get local(): AbstractControl {
    return this.form.get('local');
  }
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

  public get enableSlider(): AbstractControl {
    return this.form.get('enableSlider');
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
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.variableType = params.type;
      this._name = params?.name || '';
      this._local = params?.local==='true' || false;
      this.isEditMode = params?.isEditMode || false;
      this._value = params?.value || '';
    });

    this.form = new FormGroup({
      local: new FormControl(this._local),
      variableName: new FormControl(this._name, Validators.required),
      type: new FormControl(this.variableType, Validators.required),
      value: new FormControl(this._value),
      units: new FormControl(''),
      rotation: new FormControl(0),
      shortDescription: new FormControl(''),
      detailedDescription: new FormControl(''),
      enableSlider: new FormControl(true),
      sliderBoundsMax: new FormControl(1),
      sliderBoundsMin: new FormControl(-1),
      sliderStepSize: new FormControl(0.1),
      sliderStepRel: new FormControl(false),
    });

    this.type.value === 'constant'
      ? this.variableName.disable()
      : this.variableName.enable();

    this.type.valueChanges.pipe(takeUntil(this.destroy$)).subscribe((type) => {
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
    let item=new VariableBase(this.electronService.minsky.canvas.item);
    const local = await item.local();
    // check there is an item of the right type
    if (typeof local==='object') this.closeWindow();
    const init = await item.init();
    const units = await item.unitsStr();
    const rotation = await item.rotation();
    const tooltip = await item.tooltip();
    const detailedText = await item.detailedText();
    const enableSlider = await item.enableSlider();
    const sliderMax = Number((await item.sliderMax()).toPrecision(3));
    const sliderMin = Number((await item.sliderMin()).toPrecision(3));
    const sliderStep = Number((await item.sliderStep()).toPrecision(3));
    const sliderStepRel = await item.sliderStepRel();
    this.electronService.minsky.nameCurrentItem('editVariable');
    
    this.local.setValue(local);
    this.value.setValue(init);
    this.units.setValue(units);
    this.rotation.setValue(rotation);
    this.shortDescription.setValue(tooltip);
    this.detailedDescription.setValue(detailedText);
    this.enableSlider.setValue(enableSlider);
    this.sliderBoundsMax.setValue(sliderMax);
    this.sliderBoundsMin.setValue(sliderMin);
    this.sliderStepSize.setValue(sliderStep);
    this.sliderStepRel.setValue(sliderStepRel);
  }

  async handleSubmit() {
    this._name=this.variableName.value;
    if (this.local.value && this._name[0]===':')
      this._name=this._name.slice(1);
    else if (!this.local.value && this._name[0]!==':')
      this._name=':'+this._name;
    
    if (this.isEditMode) {
      await this.editVariable();
      return;
    }
    await this.createVariable();
  }

  saveVariableParams(item: VariableBase) {
    item.setUnits(this.units.value || '');
    item.init(this.value.value);
    item.adjustSliderBounds(); // ensure slider bounds starts with a reasonable value
    item.rotation(this.rotation.value || 0);
    item.tooltip(this.shortDescription.value);
    item.detailedText(this.detailedDescription.value);
    item.enableSlider(this.enableSlider.value);
    if (typeof this.sliderBoundsMax.value=='number') item.sliderMax(this.sliderBoundsMax.value);
    if (typeof this.sliderBoundsMin.value=='number') item.sliderMin(this.sliderBoundsMin.value);
    if (typeof this.sliderStepSize.value=='number') item.sliderStep(this.sliderStepSize.value);
    item.sliderStepRel(this.sliderStepRel.value);
    this.closeWindow();
  }

  
  editVariable() {
    this.electronService.minsky.itemFromNamedItem('editVariable');
    let item=new VariableBase(this.electronService.minsky.canvas.item);
    this.saveVariableParams(item);
    item.retype(this.type.value);
    this.electronService.minsky.canvas.renameItem(this._name);
    
  }

  createVariable() {
    this.electronService.minsky.canvas.addVariable(this._name,this.type.value);
    this.saveVariableParams(new VariableBase(this.electronService.minsky.canvas.itemFocus))
  }

  displayInitHelp() {this.electronService.displayInitHelp();}
  
  closeWindow() {
    if (this.electronService.isElectron) {
      this.windowUtilityService.closeCurrentWindowIfNotMain();
    }
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
