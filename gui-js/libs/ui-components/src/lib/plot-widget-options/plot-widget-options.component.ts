import { Component, OnDestroy, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { commandsMapping, replaceBackSlash } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-plot-widget-options',
  templateUrl: './plot-widget-options.component.html',
  styleUrls: ['./plot-widget-options.component.scss'],
})
export class PlotWidgetOptionsComponent implements OnInit, OnDestroy {
  form: FormGroup;
  itemId: number;

  public get title(): AbstractControl {
    return this.form.get('title');
  }
  public get xLabel(): AbstractControl {
    return this.form.get('xLabel');
  }
  public get yLabel(): AbstractControl {
    return this.form.get('yLabel');
  }
  public get rhsYLabel(): AbstractControl {
    return this.form.get('rhsYLabel');
  }
  public get plotType(): AbstractControl {
    return this.form.get('plotType');
  }
  public get numberOfXTicks(): AbstractControl {
    return this.form.get('numberOfXTicks');
  }
  public get numberOfYTicks(): AbstractControl {
    return this.form.get('numberOfYTicks');
  }
  public get grid(): AbstractControl {
    return this.form.get('grid');
  }
  public get subGrid(): AbstractControl {
    return this.form.get('subGrid');
  }
  public get legend(): AbstractControl {
    return this.form.get('legend');
  }
  public get xLogScale(): AbstractControl {
    return this.form.get('xLogScale');
  }
  public get yLogScale(): AbstractControl {
    return this.form.get('yLogScale');
  }

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
    });

    this.form = new FormGroup({
      title: new FormControl(''),
      xLabel: new FormControl(''),
      yLabel: new FormControl(''),
      rhsYLabel: new FormControl(''),
      plotType: new FormControl('automatic'),
      numberOfXTicks: new FormControl(null),
      numberOfYTicks: new FormControl(null),
      grid: new FormControl(false),
      subGrid: new FormControl(false),
      legend: new FormControl(false),
      xLogScale: new FormControl(false),
      yLogScale: new FormControl(false),
    });
  }

  ngOnInit() {
    (async () => {
      await this.updateFormValues();
    })();
  }

  async updateFormValues() {
    if (this.electronService.isElectron) {
      const title = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/title`,
      });
      this.title.setValue(title);

      const xLabel = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/xlabel`,
      });
      this.xLabel.setValue(xLabel);

      const yLabel = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/ylabel`,
      });
      this.yLabel.setValue(yLabel);

      const rhsYLabel = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/y1label`,
      });
      this.rhsYLabel.setValue(rhsYLabel);

      const plotType = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/plotType`,
      });
      this.plotType.setValue(plotType);

      const numberOfXTicks = await this.electronService.sendMinskyCommandAndRender(
        {
          command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/nxTicks`,
        }
      );
      this.numberOfXTicks.setValue(numberOfXTicks);

      const numberOfYTicks = await this.electronService.sendMinskyCommandAndRender(
        {
          command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/nyTicks`,
        }
      );
      this.numberOfYTicks.setValue(numberOfYTicks);

      const grid = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/grid`,
      });
      this.grid.setValue(grid);

      const subGrid = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/subgrid`,
      });
      this.subGrid.setValue(subGrid);

      const legend = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/legend`,
      });
      this.legend.setValue(legend);

      const xLogScale = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/logx`,
      });
      this.xLogScale.setValue(xLogScale);

      const yLogScale = await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/logy`,
      });
      this.yLogScale.setValue(yLogScale);
    }
  }
  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${
          this.itemId
        }/second/title "${replaceBackSlash(this.title.value)}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${
          this.itemId
        }/second/xlabel "${replaceBackSlash(this.xLabel.value)}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${
          this.itemId
        }/second/ylabel "${replaceBackSlash(this.yLabel.value)}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${
          this.itemId
        }/second/y1label "${replaceBackSlash(this.rhsYLabel.value)}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/plotType "${this.plotType.value}"`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/nxTicks ${this.numberOfXTicks.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/nyTicks ${this.numberOfYTicks.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/grid ${this.grid.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/subgrid ${this.subGrid.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/legend ${this.legend.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/logx ${this.xLogScale.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/logy ${this.yLogScale.value}`,
      });

      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/${commandsMapping.REQUEST_REDRAW_SUBCOMMAND}`,
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

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
