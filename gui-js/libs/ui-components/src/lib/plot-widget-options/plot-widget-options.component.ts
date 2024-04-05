import { Component, OnDestroy, OnInit } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { events, PlotWidget } from '@minsky/shared';
import { Subject, takeUntil } from 'rxjs';
import { MatButtonModule } from '@angular/material/button';
import { NgFor } from '@angular/common';

@Component({
    selector: 'minsky-plot-widget-options',
    templateUrl: './plot-widget-options.component.html',
    styleUrls: ['./plot-widget-options.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
      MatButtonModule,
      NgFor,
    ],
})
export class PlotWidgetOptionsComponent implements OnInit, OnDestroy {
  form: FormGroup;
  itemId: string;
  availableMarkers=[];
  horizontalMarkers=[];
  verticalMarkers=[];
  
  destroy$ = new Subject<{}>();

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
  public get numLines(): AbstractControl {
    return this.form.get('numLines');
  }
  public get plotType(): AbstractControl {
    return this.form.get('plotType');
  }
  public get barWidth(): AbstractControl {
    return this.form.get('barWidth');
  }
  public get symbolEvery(): AbstractControl {
    return this.form.get('symbolEvery');
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
  public get legendLeft(): AbstractControl {
    return this.form.get('legendLeft');
  }
  public get legendTop(): AbstractControl {
    return this.form.get('legendTop');
  }
  public get legendFontSz(): AbstractControl {
    return this.form.get('legendFontSz');
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
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.itemId = params.itemId;
    });

    this.form = new FormGroup({
      title: new FormControl(''),
      xLabel: new FormControl(''),
      yLabel: new FormControl(''),
      rhsYLabel: new FormControl(''),
      numLines: new FormControl(1),
      plotType: new FormControl('automatic'),
      barWidth: new FormControl(100),
      symbolEvery: new FormControl(1),
      numberOfXTicks: new FormControl(null),
      numberOfYTicks: new FormControl(null),
      grid: new FormControl(false),
      subGrid: new FormControl(false),
      legend: new FormControl(false),
      legendLeft: new FormControl(false),
      legendTop: new FormControl(false),
      legendFontSz: new FormControl(false),
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
      let plot=new PlotWidget(this.electronService.minsky.namedItems.elem(this.itemId));
      this.title.setValue(await plot.title());
      this.xLabel.setValue(await plot.xlabel());
      this.yLabel.setValue(await plot.ylabel());
      this.rhsYLabel.setValue(await plot.y1label());
      this.numLines.setValue(await plot.numLines());
      this.plotType.setValue(await plot.plotType());
      this.barWidth.setValue(100*(await plot.barWidth()));
      this.symbolEvery.setValue(await plot.symbolEvery());
      this.numberOfXTicks.setValue(await plot.nxTicks());
      this.numberOfYTicks.setValue(await plot.nyTicks());
      this.grid.setValue(await plot.grid());
      this.subGrid.setValue(await plot.subgrid());
      this.legend.setValue(await plot.legend());
      this.legendLeft.setValue(await plot.legendLeft());
      this.legendTop.setValue(await plot.legendTop());
      this.legendFontSz.setValue(await plot.legendFontSz());
      this.xLogScale.setValue(await plot.logx());
      this.yLogScale.setValue(await plot.logy());
      this.availableMarkers=await plot.availableMarkers();
      this.horizontalMarkers=await plot.horizontalMarkers.$properties();
      this.verticalMarkers=await plot.verticalMarkers.$properties();
    }
  }

  getMarkersSelection(id: string) {
    let markers=[];
    let selectedOptions=(document.getElementById(id) as HTMLSelectElement).selectedOptions;
    for (let i=0; i<selectedOptions.length; ++i)
      markers.push(selectedOptions[i].value);
    return markers;
  }

  openPenStyles() {
    this.electronService.send(events.CREATE_MENU_POPUP, {
      title: 'Pen Styles',
      url: '#/headless/menu/context-menu/pen-styles',
      height: 700,
      width: 350,
    });
  }
  
  async handleSave() {
    if (this.electronService.isElectron) {
      let plot=new PlotWidget(this.electronService.minsky.namedItems.elem(this.itemId));
      plot.title(this.title.value);
      plot.xlabel(this.xLabel.value);
      plot.ylabel(this.yLabel.value);
      plot.y1label(this.rhsYLabel.value);
      plot.numLines(this.numLines.value);
      plot.plotType(this.plotType.value);
      plot.barWidth(0.01*this.barWidth.value);
      plot.symbolEvery(this.symbolEvery.value);
      plot.nxTicks(this.numberOfXTicks.value);
      plot.nyTicks(this.numberOfYTicks.value);
      plot.grid(this.grid.value);
      plot.subgrid(this.subGrid.value);
      plot.legend(this.legend.value);
      plot.legendLeft(this.legendLeft.value);
      plot.legendTop(this.legendTop.value);
      plot.legendFontSz(this.legendFontSz.value);
      plot.logx(this.xLogScale.value);
      plot.logy(this.yLogScale.value);
      plot.horizontalMarkers.$properties(this.getMarkersSelection("horizontalMarkers"));
      plot.verticalMarkers.$properties(this.getMarkersSelection("verticalMarkers"));
      plot.requestRedraw();
      this.electronService.minsky.canvas.requestRedraw();
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
