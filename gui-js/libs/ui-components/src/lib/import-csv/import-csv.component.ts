import { AfterViewInit, Component, OnDestroy, OnInit } from '@angular/core';
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
  checkboxes: Array<boolean> = [];
  spec: Record<string, unknown>;
  initialDimensionNames: string[];

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
      this.variableValuesSubCommand = `/minsky/variableValues/@elem/${this.valueId}/second`;

      await this.getCSVDialogSpec();
      this.updateForm();
      this.selectedHeader = this.spec.headerRow as number;

      this.setupListenerForCleanup();
    })();
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
    this.columnar.setValue(this.spec.columnar);
    this.decSeparator.setValue(this.spec.decSeparator);
    this.duplicateKeyAction.setValue(this.spec.duplicateKeyAction);
    this.escape.setValue(this.spec.escape);
    this.horizontalDimName.setValue(this.spec.horizontalDimName);
    this.mergeDelimiters.setValue(this.spec.mergeDelimiters);
    this.missingValue.setValue(this.spec.missingValue);
    this.quote.setValue(this.spec.quote);
    this.separator.setValue(this.spec.separator);
  }

  async getValueId() {
    const command = `/minsky/namedItems/@elem/${this.itemId}/second/valueId`;

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
    // await this.redraw();
  }

  async getCSVDialogSpec() {
    this.spec = (await this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog/spec`,
    })) as Record<string, unknown>;
    this.initialDimensionNames = this.spec.dimensionNames as string[];

    console.log('🚀 this.CSVDialogSpec', this.spec);
  }

  async parseLines() {
    this.parsedLines = (await this.electronService.sendMinskyCommandAndRender({
      command: `${this.variableValuesSubCommand}/csvDialog/parseLines`,
    })) as string[][];

    this.csvCols = new Array(this.parsedLines[0]?.length);
    this.checkboxes = new Array(this.parsedLines[0]?.length - 1).fill(false);
  }

  async selectHeader(index: number) {
    this.selectedHeader = index;

    this.spec.headerRow = this.selectedHeader;

    // await this.parseLines();
  }

  async selectRowAndCol(rowIndex: number, colIndex: number) {
    this.selectedRow = rowIndex;
    this.spec.dataRowOffset = rowIndex;

    this.selectedCol = colIndex;
    this.spec.dataColOffset = colIndex;

    for (let i = this.selectedCol + 1; i <= this.parsedLines.length - 1; i++) {
      this.checkboxes[i] = false;
    }
  }

  getColorForCell(rowIndex: number, colIndex: number) {
    let color = '';

    if (this.selectedHeader === rowIndex) {
      //header row
      color = 'blue';
      if (this.selectedCol >= 0 && this.selectedCol > colIndex) {
        color = 'green';
      }
    } else {
      //not a header row
      if (this.selectedCol >= 0 && this.selectedCol > colIndex) {
        // column
        color =
          this.checkboxes[colIndex] && this.selectedHeader !== rowIndex
            ? 'blue'
            : 'red';
      }

      if (this.selectedRow >= 0 && this.selectedRow > rowIndex) {
        // row
        color = 'red';
      }
    }

    return color;
  }

  updateDimColsAndNames() {
    this.spec.dimensionCols = this.checkboxes
      .map((c, index) => (c ? index : false))
      .filter((v) => v !== false);

    this.spec.dimensionNames = this.parsedLines[
      this.selectedHeader
    ].filter((value, index) =>
      (this.spec.dimensionCols as number[]).includes(index)
    );
  }

  updatedCheckBoxValue(event: any, index: number) {
    this.checkboxes[index] = event.target.checked;
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
      ...this.spec,
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
    };
    console.log(
      '🚀 ~ file: import-csv.component.ts ~ line 327 ~ ImportCsvComponent ~ handleSubmit ~ spec',
      spec
    );
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
      // this.closeWindow();

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

    // this.closeWindow();
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
