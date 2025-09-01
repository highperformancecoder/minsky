import { AfterViewInit, AfterViewChecked, Component, ChangeDetectorRef, OnDestroy, OnInit, ElementRef, ViewChild } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import {
  CSVDialog,
  dateTimeFormats,
  events,
  importCSVvariableName,
  VariableBase,
  VariableValue,
  Zoomable,
} from '@minsky/shared';
import { MessageBoxSyncOptions } from 'electron/renderer';
import { OpenDialogOptions } from 'electron';
import { Subject, takeUntil } from 'rxjs';
import { NgStyle } from '@angular/common';
import { MatOptionModule } from '@angular/material/core';
import { MatAutocompleteModule } from '@angular/material/autocomplete';
import { MatButtonModule } from '@angular/material/button';
import JSON5 from 'json5';

enum ColType {
  axis = "axis",
  data = "data",
  ignore = "ignore"
};

function separatorToChar(sep: string): string {
  switch (sep) {
    case 'space': return ' ';
    case 'tab': return '\t';
    default: return sep;
  }
}

function separatorFromChar(sep: string): string {
  switch (sep) {
    case ' ': return 'space';
    case '\t': return 'tab';
    default: return sep;
  }
}

class Dimension {
  type: string;
  units: string;
};

@Component({
  selector: 'minsky-import-csv',
  templateUrl: './import-csv.component.html',
  styleUrls: ['./import-csv.component.scss'],
  standalone: true,
  imports: [
    FormsModule,
    ReactiveFormsModule,
    MatButtonModule,
    MatAutocompleteModule,
    MatOptionModule,
    NgStyle
],
})
export class ImportCsvComponent extends Zoomable implements OnInit, AfterViewInit, AfterViewChecked, OnDestroy {
  form: FormGroup;

  destroy$ = new Subject<{}>();

  tabs = [
    {
      index: 0,
      caption: 'File selection',
      disabled: false
    },
    {
      index: 1,
      caption: 'CSV settings',
      disabled: true
    },
    {
      index: 2,
      caption: 'Data selection',
      disabled: true
    },
    {
      index: 3,
      caption: 'Import settings',
      disabled: true
    }];

  selectedTabIndex = 0;

  fileLoaded = false;

  systemWindowId: number;
  isInvokedUsingToolbar: boolean;
  newTable: boolean;
  examplesPath: string;
  csvDialog: CSVDialog;
  timeFormatStrings = dateTimeFormats;
  parsedLines: string[][] = [];
  csvCols: any[];
  colType: Array<ColType> = [];
  selected: boolean[]; ///< per column whether column is selected
  mouseDown = -1;       ///< record of column of previous mouseDown
  dialogState: any;
  uniqueValues = [];
  files: string[] = [];


  existingDimensionNames: string[] = [];
  selectableDimensionNames: string[][] = [];
  wedgeOptionPanelVisibleIndex: number = null;
  @ViewChild('checkboxRow') checkboxRow: ElementRef<HTMLCollection>;
  @ViewChild('importCsvCanvasContainer') inputCsvCanvasContainer: ElementRef<HTMLElement>;
  @ViewChild('fullDialog') fullDialog: ElementRef<HTMLElement>;

  public get parameterName(): AbstractControl {
    return this.form.get('parameterName');
  }
  public get shortDescription(): AbstractControl {
    return this.form.get('shortDescription');
  }
  public get detailedDescription(): AbstractControl {
    return this.form.get('detailedDescription');
  }
  public get url(): AbstractControl {
    return this.form.get('url');
  }
  public get dontFail(): AbstractControl {
    return this.form.get('dontFail');
  }
  public get counter(): AbstractControl {
    return this.form.get('counter');
  }
  public get dropTable(): AbstractControl {
    return this.form.get('dropTable');
  }
  public get separator(): AbstractControl {
    return this.form.get('separator');
  }
  public get decSeparator(): AbstractControl {
    return this.form.get('decSeparator');
  }
  public get escape(): AbstractControl {
    return this.form.get('escape');
  }
  public get quote(): AbstractControl {
    return this.form.get('quote');
  }
  public get mergeDelimiters(): AbstractControl {
    return this.form.get('mergeDelimiters');
  }
  public get missingValue(): AbstractControl {
    return this.form.get('missingValue');
  }
  public get duplicateKeyAction(): AbstractControl {
    return this.form.get('duplicateKeyAction');
  }
  public get horizontalDimName(): AbstractControl {
    return this.form.get('horizontalDimName');
  }
  public get horizontalDimension() {
    return this.form.get('horizontalDimension') as FormGroup<any>;
  }
  public get type(): AbstractControl {
    return this.horizontalDimension.get('type');
  }
  public get format(): AbstractControl {
    return this.horizontalDimension.get('units');
  }

  onTabClick(index: number) {
    if (!this.tabs[index].disabled) this.selectedTabIndex = index;
  }

  zoom(ratio: number) {
    this.zoomFactor *= ratio;
    let style = this.fullDialog.nativeElement.style;
    style.setProperty('zoom', `${Math.round(this.zoomFactor * 100)}%`);
    style.setProperty('width', `${Math.round(100 / this.zoomFactor)}vw`);
    style.setProperty('height', `${Math.round(100 / this.zoomFactor)}vh`);
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private cdr: ChangeDetectorRef
  ) {
    super();
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.csvDialog = new CSVDialog(params.csvDialog);
      this.systemWindowId = params.systemWindowId;
      this.isInvokedUsingToolbar = params.isInvokedUsingToolbar==="true";
      this.newTable = params.dropTable==="true";
      this.examplesPath = params.examplesPath;
    });

    this.form = new FormGroup({
      parameterName: new FormControl(''),
      shortDescription: new FormControl(''),
      detailedDescription: new FormControl(''),
      dontFail: new FormControl(false),
      counter: new FormControl(false),
      dropTable: new FormControl(false),
      decSeparator: new FormControl('.'),
      duplicateKeyAction: new FormControl('throwException'),
      escape: new FormControl(''),
      horizontalDimName: new FormControl('?'),
      mergeDelimiters: new FormControl(false),
      missingValue: new FormControl(NaN),
      quote: new FormControl('"'),
      separator: new FormControl(','),

      // review
      url: new FormControl(''),
      horizontalDimension: new FormGroup({
        type: new FormControl('string'),
        units: new FormControl(''),
      }),
    });

    this.electronService.on(events.REFRESH_CSV_IMPORT, async e => {
      await this.getCSVDialogSpec();
      this.updateColumnTypes();
      this.updateForm();
      this.cdr.detectChanges();
    });
  }

  ngOnInit() {
    this.form.valueChanges.pipe(takeUntil(this.destroy$)).subscribe(async (_form) => {
      if (this.url === _form.url) {
        await this.parseLines();
      }
    });
    document.onkeydown = this.onKeyDown;

    this.electronService.minsky.dimensions.$properties().then(dims => {
      this.existingDimensionNames = Object.keys(dims);
    });
  }

  ngAfterViewInit() {
    (async () => {
      await this.getCSVDialogSpec();
      this.updateForm();
      this.load(2);
      this.selectRowAndCol(this.dialogState.spec.dataRowOffset, this.dialogState.spec.dataColOffset);
    })();
  }

  ngAfterViewChecked() {
    // set state of dimension controls to reflect dialogState
    if (this.inputCsvCanvasContainer) {
      var table = this.inputCsvCanvasContainer.nativeElement.children[0] as HTMLTableElement;
      if (!table) return;
      for (var i = 0; i < this.colType.length; ++i) {
        var colType = table.rows[2].cells[i + 1]?.children[0] as HTMLInputElement;
        if (colType)
          colType.value = this.colType[i];
        if (this.colType[i] === ColType.axis) {
          var type = table.rows[3].cells[i + 1]?.children[0] as HTMLSelectElement;
          var dimension = this.dialogState.spec.dimensions[i];
          if (!dimension) dimension = { type: "string", units: "" };
          type.value = dimension.type;
        }
      }
    }
  }


  updateForm() {
    this.url.setValue(this.dialogState.url);
    if (!this.files && this.dialogState.url) this.files=[this.dialogState.url];

    this.dontFail.setValue(this.dialogState.spec.dontFail);
    this.counter.setValue(this.dialogState.spec.counter);
    this.decSeparator.setValue(this.dialogState.spec.decSeparator);
    this.duplicateKeyAction.setValue(this.dialogState.spec.duplicateKeyAction);
    this.escape.setValue(this.dialogState.spec.escape);
    this.horizontalDimName.setValue(this.dialogState.spec.horizontalDimName);
    this.mergeDelimiters.setValue(this.dialogState.spec.mergeDelimiters);
    this.missingValue.setValue(this.dialogState.spec.missingValue);
    this.quote.setValue(this.dialogState.spec.quote);
    this.separator.setValue(separatorFromChar(this.dialogState.spec.separator));
    this.horizontalDimension.setValue({
      type: this.dialogState.spec.horizontalDimension.type,
      units: this.dialogState.spec.horizontalDimension.units
    });
  }

  async selectFile(defaultPath: string = '') {
    let options: OpenDialogOptions = {
      defaultPath: ':data',
      filters: [
        { extensions: ['csv'], name: 'CSV' },
        { extensions: ['*'], name: 'All Files' },
      ],
      properties: ['openFile', 'multiSelections'],
    };
    // support examples directory
    if (defaultPath) options['defaultPath'] = defaultPath;
    const filePaths = await this.electronService.openFileDialog(options);

    if (!filePaths) return;
    if (typeof filePaths == 'string') {
      this.files = [filePaths];
      this.url.setValue(filePaths);
      this.dialogState.url = filePaths;
    } else {
      this.files = filePaths;
      this.url.setValue(filePaths[0]);
      this.dialogState.url = filePaths[0];
    }

    await this.load(2);
  }

  async load(selectTab) {
    if (this.url.value === '') return;

    this.setParameterNameFromUrl();

    if (this.url.value.includes('://')) {
      const savePath = await this.electronService.downloadCSV({ windowUid: this.csvDialog.$prefix(), url: this.url.value });
      this.url.setValue(savePath);
      this.files = [savePath];
    }

    const fileUrlOnServer = await this.csvDialog.url();

    if (!this.url.value || this.url.value !== fileUrlOnServer) {
      await this.csvDialog.url(this.url.value);
      await this.csvDialog.guessSpecAndLoadFile();
      await this.getCSVDialogSpec();
      this.updateForm();
    }
    await this.csvDialog.loadFile();
    await this.parseLines();

    for (const tab of this.tabs) {
      tab.disabled = false;
    }

    this.fileLoaded = true;

    this.selectedTabIndex = selectTab;
  }

  setParameterNameFromUrl() {
    const path = this.url.value as string;
    const pathArray = this.electronService.isWindows() ? path.split(`\\`) : path.split(`/`);
    const fileName = pathArray[pathArray.length - 1].split(`.`)[0];
    this.parameterName.setValue(fileName);
  }

  async getCSVDialogSpec() {
    this.csvDialog.spec.toSchema();
    this.dialogState = await this.csvDialog.$properties() as Record<string, unknown>;
    this.uniqueValues = await this.csvDialog.correctedUniqueValues();
  }

  updateColumnTypes() {
    this.colType = new Array(this.parsedLines[this.dialogState.spec.headerRow]?.length).fill("ignore");
    for (var i in this.dialogState.spec.dimensionCols as Array<number>) {
      var col = this.dialogState.spec.dimensionCols[i];
      if (col < this.colType.length)
        this.colType[col] = ColType.axis;
    }
    if (this.dialogState.spec.dataCols)
      for (var i in this.dialogState.spec.dataCols as Array<number>) {
        var col = this.dialogState.spec.dataCols[i];
        if (col < this.colType.length)
          this.colType[col] = ColType.data;
      }
    else
      // emulate old behaviour, if no datacols specified, all columns to the right of dataColOffset are data
      for (var ii = this.dialogState.dataColOffset as number; ii < this.colType.length; ++ii)
        this.colType[ii] = ColType.data;
  }

  async parseLines() {
    this.parsedLines = await this.csvDialog.parseLines(this.dialogState.spec.maxColumn) as string[][];
    await this.getCSVDialogSpec();

    let header = this.dialogState.spec.headerRow;
    this.csvCols = new Array(this.dialogState.spec.numCols);
    this.selected = new Array(this.dialogState.spec.numCols).fill(false);
    this.selectableDimensionNames = this.parsedLines[header] ? this.parsedLines[header].map(header => this.getSelectableNameDimensions(header)) : [];
    this.updateColumnTypes();
  }

  hypercubeSize() {
    let r = 1;
    let dataDims = 0;
    for (let i in this.uniqueValues)
      switch (this.colType[i]) {
        case ColType.axis:
          r *= this.uniqueValues[i];
          break;
        case ColType.data:
          dataDims++;
          break;
      }
    if (dataDims > 0)
      return r * dataDims;
    return r;
  }

  onSeparatorChange() {
    this.updateSpecFromForm();
    this.csvDialog.spec.$properties(this.dialogState.spec);
    this.parseLines();
  }

  async selectHeader(index: number) {
    this.dialogState.spec.headerRow = index;
    // explicitly selecting a header, make sure dataRowOffset is greater
    if (this.dialogState.spec.dataRowOffset <= index)
      this.dialogState.spec.dataRowOffset = index + 1;
  }

  async selectRowAndCol(rowIndex: number, colIndex: number) {
    this.dialogState.spec.dataRowOffset = rowIndex;
    if (this.dialogState.spec.headerRow >= rowIndex)
      this.dialogState.spec.headerRow = rowIndex - 1;


    this.dialogState.spec.dataColOffset = colIndex;

    if (!this.parsedLines.length) return;
    for (let i = 0; i < this.parsedLines[0].length; i++)
      if (i < colIndex) {
        if (this.colType[i] == ColType.data)
          this.colType[i] = ColType.ignore;
      } else
        this.colType[i] = ColType.data;
    this.ngAfterViewChecked();
  }

  getColorForCell(rowIndex: number, colIndex: number) {
    if (colIndex >= this.colType.length) return "red";

    if (this.dialogState.spec.headerRow === rowIndex && this.dialogState.spec.headerRow < this.dialogState.spec.dataRowOffset)  // header row
      switch (this.colType[colIndex]) {
        case 'data': return "blue";
        case 'axis': return "green";
        case "ignore": return "red";
      }
    else if (this.dialogState.spec.dataRowOffset >= 0 && this.dialogState.spec.dataRowOffset > rowIndex)
      return "red"; // ignore commentary at beginning of file
    else
      switch (this.colType[colIndex]) {
        case 'data': return "black";
        case 'axis': return "blue";
        case "ignore": return "red";
      }
    return 'black';
  }

  setColTypeImpl(column: number, type: ColType) {
    this.colType[column] = type;
    if (!this.dialogState.spec.dimensionNames[column])
      this.dialogState.dimensionNames[column] = this.parsedLines[this.dialogState.spec.headerRow][column];
    if (!this.dialogState.spec.dimensions[column])
      this.dialogState.spec.dimensions[column] = { type: "string", units: "" } as Dimension;
  }

  setColType(column: number, type: ColType) {
    if (this.selected.every((x) => !x)) { // nothing selected
      this.setColTypeImpl(column, type);
      return;
    }

    for (let i = 0; i < this.selected.length; ++i)
      if (this.selected[i]) {
        this.setColTypeImpl(i, type);
      }
    this.selected.fill(false);
    this.cdr.detectChanges();
  }

  typeMouseDown(event: any, col: number) {
    if (event?.button === 0)
      this.mouseDown = col;
  }

  typeMouseMove(event: any, col: number) {
    if (event?.button === 0 && this.mouseDown >= 0 && col !== this.mouseDown) {
      if (col < this.mouseDown) [col, this.mouseDown] = [this.mouseDown, col];
      for (let i = 0; i < this.selected.length; ++i)
        this.selected[i] = i >= this.mouseDown && i <= col;
    }
  }

  typeMouseUp(event: any, row: any, col: number) {
    if (event?.button === 0) {
      this.typeMouseMove(event, col);
      if (col === this.mouseDown)  // deselect all if ending on same column
        if (this.selected.every((x) => !x)) {
          // hide selectRowAndColumn behind a modifier key to prevent accidental settings.
          if (Number.isInteger(row) && (event.shiftKey || event.ctrlKey || event.altKey || event.metaKey))
            this.selectRowAndCol(row, col);
          else
            this.dialogState.spec.dimensionNames[col] = this.parsedLines[this.dialogState.spec.headerRow][col]
        }
        else
          this.selected.fill(false);
      this.cdr.detectChanges();
      this.mouseDown = -1;
    }
  }

  updateSpecFromForm() {
    var spec = this.dialogState.spec;
    spec.dontFail = this.dontFail.value;
    spec.counter = this.counter.value;
    spec.decSeparator = this.decSeparator.value;
    spec.duplicateKeyAction = this.duplicateKeyAction.value;
    spec.escape = this.escape.value;
    spec.horizontalDimName = this.horizontalDimName.value;
    spec.mergeDelimiters = this.mergeDelimiters.value;
    spec.missingValue = this.missingValue.value;
    spec.quote = this.quote.value;
    spec.separator = separatorToChar(this.separator.value);
    spec.horizontalDimension = this.horizontalDimension.value;

    this.dialogState.spec.dimensionCols = [];
    this.dialogState.spec.dataCols = [];
    if (this.dialogState.spec.dataColOffset < this.colType.length)
      this.dialogState.spec.dataColOffset = this.colType.length;
    for (let i = 0; i < this.colType.length; ++i)
      switch (this.colType[i]) {
        case ColType.axis:
          this.dialogState.spec.dimensionCols.push(i);
          break;
        case ColType.data:
          this.dialogState.spec.dataCols.push(i);
          break;
      }
  }

  async handleSubmit() {
    if (this.parameterName.value === '') {
      this.setParameterNameFromUrl();
    }

    this.updateSpecFromForm();

    if (this.dialogState.spec.dataCols.length === 0)
      this.dialogState.spec.counter = true;
    this.csvDialog.spec.$properties(this.dialogState.spec);
    
    if (!this.files || !this.files[0]) this.files=[this.url.value];
    if (this.files && (this.dropTable.value || this.newTable))
      this.electronService.minsky.databaseIngestor.createTable(this.files[0]);
    // returns an error message on error
    const res = await this.csvDialog.importFromCSV(this.files) as unknown as string;

    if (typeof res === 'string') {
      const positiveResponseText = 'Yes';
      const negativeResponseText = 'No';

      const options: MessageBoxSyncOptions = {
        buttons: [positiveResponseText, negativeResponseText],
        message: `Something went wrong: ${res}\nDo you want to generate a report?`,
        title: 'Generate Report ?',
      };

      const index = await this.electronService.showMessageBoxSync(options);

      if (options.buttons[index] === positiveResponseText) {
        await this.doReport();
      }
      return;
    }

    if (this.isInvokedUsingToolbar && this.parameterName.value) {
      // rename variable if newly added variable is still focussed
      let vv=new VariableValue(this.csvDialog.$prefix());
      let v=new VariableBase(this.electronService.minsky.canvas.itemFocus);
      let vvId=await vv?.valueId();
      if (await v?.valueId()!==vvId) {// this is not a focussed item
        v=new VariableBase(this.electronService.minsky.canvas.item); // item has been clicked
        if (await v?.valueId()!==vvId)
          v=null;
      }
      if (v) {
        v.name(this.parameterName.value);
        v.tooltip(this.shortDescription.value);
        v.detailedText(this.detailedDescription.value);
      }
    }
    this.closeWindow();
  }

  async doReport() {
    const filePathWithoutExt = (this.url.value as string)
      .toLowerCase()
      .split('.csv')[0];

    const filePath = await this.electronService.saveFileDialog({
      defaultPath: `:data/${filePathWithoutExt}-error-report.csv`,
      title: 'Save report',
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [{ extensions: ['csv'], name: 'CSV' }],
    });
    if (!filePath) return;

    await this.csvDialog.reportFromFile(this.url.value, filePath);
    return;
  }

  async contextMenu(row: number, col: number) {
    // update C++ spec with current state
    this.updateSpecFromForm();
    await this.csvDialog.spec.$properties(this.dialogState.spec);
    this.electronService.send(events.CONTEXT_MENU, {
      x: row,
      y: col,
      type: 'csv-import',
      command: this.csvDialog.$prefix(),
    });
  }

  // creates list of selectable name dimensions, adding header value to existing dimensions if necessary
  // should not be called from HTML template because this will create a massive number of arrays that have to be garbage-collected
  getSelectableNameDimensions(headerValue: string) {
    const selectableDimensions = this.existingDimensionNames.slice();
    if (!selectableDimensions.includes(headerValue)) selectableDimensions.push(headerValue);
    return selectableDimensions;
  }

  onWedgeIconClicked($event, i) {
    if (this.wedgeOptionPanelVisibleIndex === i) {
      this.wedgeOptionPanelVisibleIndex = null;
    } else {
      this.wedgeOptionPanelVisibleIndex = i;
    }
    $event.stopPropagation();
  }

  onWedgeOptionClicked($event, i, value) {
    this.wedgeOptionPanelVisibleIndex = null;
    if (i === -1) {
      this.form.controls.horizontalDimName.setValue(value);
    } else {
      this.dialogState.spec.dimensionNames[i] = value;
    }
    $event.stopPropagation();
  }

  next() {
    this.selectedTabIndex++;
  }

  closeWindow() { this.electronService.closeWindow(); }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
