import { AfterViewInit, Component, OnDestroy, OnInit, ElementRef, ViewChild } from '@angular/core';
import { AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import {
  commandsMapping,
  dateTimeFormats,
  importCSVerrorMessage,
  importCSVvariableName,
  isWindows,
  normalizeFilePathForPlatform,
} from '@minsky/shared';
import { MessageBoxSyncOptions } from 'electron/renderer';
import * as JSON5 from 'json5';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

enum ColType {
  axis="axis",
  data="data",
  ignore="ignore"
};

function separatorToChar(sep: string): string {
  switch(sep) {
  case 'space': return ' ';
  case 'tab': return '\t';
  default: return sep;
  }
}

function separatorFromChar(sep: string): string {
  switch(sep) {
  case ' ': return 'space';
  case '\t': return 'tab';
  default: return sep;
  }
}

@AutoUnsubscribe()
@Component({
  selector: 'minsky-import-csv',
  templateUrl: './import-csv.component.html',
  styleUrls: ['./import-csv.component.scss'],
})
export class ImportCsvComponent implements OnInit, AfterViewInit, OnDestroy {
  form: FormGroup;

  itemId: number;
  systemWindowId: number;
  isInvokedUsingToolbar: boolean;
  valueId: number;
  variableValuesSubCommand: string;
  timeFormatStrings = dateTimeFormats;
  parsedLines: string[][] = [];
  csvCols: any[];
  selectedHeader = 0;
  selectedRow = -1;
  selectedCol = -1;
  colType: Array<ColType> = [];
  dialogState: any;
  initialDimensionNames: string[];
  @ViewChild('colTypeRow') colTypeRow: ElementRef<HTMLCollection>;
  @ViewChild('importCsvCanvasContainer') inputCsvCanvasContainer: ElementRef<HTMLElement>;

  public get url(): AbstractControl {
    return this.form.get('url');
  }
  public get columnar(): AbstractControl {
    return this.form.get('columnar');
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
  public get horizontalDimension(): AbstractControl {
    return this.form.get('horizontalDimension');
  }
  public get type(): AbstractControl {
    return this.horizontalDimension.get('type');
  }
  public get format(): AbstractControl {
    return this.horizontalDimension.get('units');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
      this.isInvokedUsingToolbar = params.isInvokedUsingToolbar;
    });

    this.form = new FormGroup({
      columnar: new FormControl(false),
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
  }

  ngOnInit() {
    // ??
    this.form.valueChanges.subscribe(async (_form) => {
      if (this.url === _form.url) {
        await this.parseLines();
      }
    });
  }

  ngAfterViewInit() {
    (async () => {
      this.valueId = await this.getValueId();
      this.variableValuesSubCommand = `/minsky/variableValues/@elem/${JSON5.stringify(this.valueId)}/second`;

      await this.getCSVDialogSpec();
      this.updateForm();
      this.selectedHeader = this.dialogState.spec.headerRow as number;
      this.load();
      this.selectRowAndCol(this.dialogState.spec.dataRowOffset, this.dialogState.spec.dataColOffset);
      this.setupListenerForCleanup();
    })();
  }
  ngAfterViewChecked() 	{
    // set state of dimension controls to reflect dialogState
    if (this.inputCsvCanvasContainer)
    {
      var table=this.inputCsvCanvasContainer.nativeElement.children[0] as HTMLTableElement;
      for (var i=0; i<this.colType.length; ++i)
        {
            var colType=table.rows[0].cells[i+1].children[0] as HTMLInputElement;
            if (colType)
                colType.value=this.colType[i];
            if (this.colType[i]===ColType.axis)
            {
                var type=table.rows[1].cells[i+1].children[0] as HTMLSelectElement;
                type.value=this.dialogState.spec.dimensions[i].type;
                var format=table.rows[2].cells[i+1].children[0] as HTMLInputElement;
                format.value=this.dialogState.spec.dimensions[i].units;
                var name=table.rows[3].cells[i+1].children[0] as HTMLInputElement;
                name.value=this.dialogState.spec.dimensionNames[i];
            }
        }
    }
  }
  
  private setupListenerForCleanup() {
    this.electronService.remote.getCurrentWindow().on('close', async () => {
      if (this.isInvokedUsingToolbar) {
        const currentItemId = await this.electronService.sendMinskyCommandAndRender(
          {
            command: commandsMapping.CANVAS_ITEM_ID,
          }
        );
        const currentItemName = await this.electronService.sendMinskyCommandAndRender(
          {
            command: commandsMapping.CANVAS_ITEM_NAME,
          }
        );
        // We do this check because item focus might have changed when importing csv if user decided to work on something else

        if (
          currentItemId === this.itemId &&
          currentItemName === importCSVvariableName
        ) {
          await this.electronService.sendMinskyCommandAndRender({
            command: commandsMapping.CANVAS_DELETE_ITEM,
          });
        }
      }
    });
  }

  updateForm() {
    this.url.setValue(this.dialogState.url);
    
    this.columnar.setValue(this.dialogState.spec.columnar);
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
      units:this.dialogState.spec.horizontalDimension.units
    });
  }

  async getValueId() {
    const command = `/minsky/namedItems/@elem/"${this.itemId}"/second/valueId`;

    const valueId = (await this.electronService.sendMinskyCommandAndRender({
      command,
    })) as number;

    return valueId;
  }

  async selectFile() {
    const fileDialog = await this.electronService.remote.dialog.showOpenDialog({
      filters: [
        { extensions: ['csv'], name: 'CSV' },
        { extensions: ['*'], name: 'All Files' },
      ],
    });

    if (fileDialog.canceled || !fileDialog.filePaths) {
      return;
    }

    const filePath = fileDialog.filePaths[0].toString();

    this.url.setValue(filePath);
    this.dialogState.url=filePath;
  }

  async load() {
    const fileUrlOnServer = (await this.electronService.sendMinskyCommandAndRender(
      {
        command: `${this.variableValuesSubCommand}/csvDialog/url`,
      }
    )) as string;

    const fileUrl = this.url.value;

    if (fileUrl !== fileUrlOnServer) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${
          this.variableValuesSubCommand
        }/csvDialog/url ${normalizeFilePathForPlatform(fileUrl)}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${this.variableValuesSubCommand}/csvDialog/guessSpecAndLoadFile`,
      });
    } else {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${this.variableValuesSubCommand}/csvDialog/loadFile`,
      });
    }

    await this.parseLines();
  }

  async getCSVDialogSpec() {
    this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog/spec/toSchema`,
    });
    this.dialogState = (await this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog`,
    })) as Record<string, unknown>;
    this.initialDimensionNames = this.dialogState.spec.dimensionNames as string[];
  }

  async parseLines() {
    this.parsedLines = (await this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog/parseLines`,
    })) as string[][];

    this.csvCols = new Array(this.parsedLines[0]?.length);
    this.colType = new Array(this.parsedLines[0]?.length).fill("ignore");
    for (var i in this.dialogState.spec.dimensionCols as Array<number>)
    {
      var col=this.dialogState.spec.dimensionCols[i];
      if (col<this.colType.length)
        this.colType[col]=ColType.axis;
    }
    if (this.dialogState.spec.dataCols)
      for (var i in this.dialogState.spec.dataCols as Array<number>)
    {
      var col=this.dialogState.spec.dimensionCols[i];
      if (col<this.colType.length)
        this.colType[col]=ColType.data;
    }
    else
      // emulate old behaviour, if no datacols specified, all columns to the right of dataColOffset are data
      for (var ii=this.dialogState.dataColOffset as number; ii<this.colType.length; ++ii)
        this.colType[ii]=ColType.data;
    
    this.updateDimColsAndNames();
  }
  
  async selectHeader(index: number) {
    this.selectedHeader = index;
    this.dialogState.spec.headerRow = this.selectedHeader;
  }

  async selectRowAndCol(rowIndex: number, colIndex: number) {
    this.selectedRow = rowIndex;
    this.dialogState.spec.dataRowOffset = rowIndex;

    this.selectedCol = colIndex;
    this.dialogState.spec.dataColOffset = colIndex;

    for (let i = 0; i<this.parsedLines[0].length; i++)
      //for (let i = this.selectedCol; this.dialogState.spec.columnar? i<this.selectedCol + 1: i < this.parsedLines.length; i++) {
      if (i<this.selectedCol) {
        if (this.colType[i]==ColType.data)
          this.colType[i]=ColType.ignore;
      } else if (i===this.selectedCol || !this.columnar.value)
        this.colType[i]=ColType.data;
    else
      this.colType[i]=ColType.ignore;
    this.ngAfterViewChecked();
  }

  getColorForCell(rowIndex: number, colIndex: number) {
    if (colIndex>=this.colType.length) return "red";

    if (this.selectedHeader === rowIndex)  // header row
        switch (this.colType[colIndex]) {
        case 'data': return "blue";
        case 'axis': return "green";
        case "ignore": return "red";
        }
    else if (this.selectedRow >= 0 && this.selectedRow > rowIndex)
      return "red"; // ignore commentary at beginning of file
    else
      switch (this.colType[colIndex]) {
      case 'data': return "black";
      case 'axis': return "blue";
      case "ignore": return "red";
      }
  }

  updateDimColsAndNames() {
    this.dialogState.spec.dimensionCols = this.colType
      .map((c, index) => (c===ColType.axis ? index : false))
      .filter((v) => v !== false);
    this.dialogState.spec.dataCols = this.colType
      .map((c, index) => (c===ColType.data ? index : false))
      .filter((v) => v !== false);

    this.dialogState.spec.dimensionNames = this.parsedLines[
      this.selectedHeader
    ].filter((value, index) =>
      (this.dialogState.spec.dimensionCols as number[]).includes(index)
    );
  }

  setColType(column: number, type: ColType) {
    this.colType[column]=type;
    this.updateDimColsAndNames();
  }

  async handleSubmit() {
    const {
      columnar,
      decSeparator,
      duplicateKeyAction,
      escape,
      horizontalDimName,
      mergeDelimiters,
      missingValue,
      quote,
      separator,
      horizontalDimension,
    } = this.form.value;

    const spec = {
      ...this.dialogState.spec,
      columnar,
      decSeparator,
      duplicateKeyAction,
      escape,
      horizontalDimName,
      mergeDelimiters,
      missingValue,
      quote,
      separator: separatorToChar(separator),
      horizontalDimension,
    };
    const res = await this.electronService.sendMinskyCommandAndRender({
      command: `${
        commandsMapping.CANVAS_ITEM_IMPORT_FROM_CSV
      } [${normalizeFilePathForPlatform(this.url.value)},${JSON5.stringify(
        spec
      )}]`,
    });

    if (res === importCSVerrorMessage) {
      const positiveResponseText = 'Yes';
      const negativeResponseText = 'No';

      const options: MessageBoxSyncOptions = {
        buttons: [positiveResponseText, negativeResponseText],
        message: 'Something went wrong... Do you want to generate a report?',
        title: 'Generate Report ?',
      };

      const index = this.electronService.remote.dialog.showMessageBoxSync(
        options
      );

      if (options.buttons[index] === positiveResponseText) {
        await this.doReport();
      }
      this.closeWindow();

      return;
    }

    const currentItemId = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.CANVAS_ITEM_ID,
      }
    );
    const currentItemName = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.CANVAS_ITEM_NAME,
      }
    );

    if (
      this.isInvokedUsingToolbar &&
      currentItemId === this.itemId &&
      currentItemName === importCSVvariableName &&
      this.url.value
    ) {
      const path = this.url.value as string;
      const pathArray = isWindows() ? path.split(`\\`) : path.split(`/`);

      const fileName = pathArray[pathArray.length - 1].split(`.`)[0];

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.RENAME_ITEM} "${fileName}"`,
      });
    }

    this.closeWindow();
  }

  async doReport() {
    const filePathWithoutExt = (this.url.value as string)
      .toLowerCase()
      .split('.csv')[0];

    const {
      canceled,
      filePath: _filePath,
    } = await this.electronService.remote.dialog.showSaveDialog({
      defaultPath: `${filePathWithoutExt}-error-report.csv`,
      title: 'Save report',
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [{ extensions: ['csv'], name: 'CSV' }],
    });

    const filePath = normalizeFilePathForPlatform(_filePath);
    const url = normalizeFilePathForPlatform(this.url.value);
    if (canceled || !filePath) {
      return;
    }

    await this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog/reportFromFile [${url},${filePath}]`,
    });

    return;
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
