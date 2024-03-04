import { Component, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { PlotWidget } from '@minsky/shared';
import { fromEvent, Observable, Subject, takeUntil } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@Component({
    selector: 'minsky-plot-widget-view',
    templateUrl: './plot-widget-view.component.html',
    styleUrls: ['./plot-widget-view.component.scss'],
    standalone: true,
})
export class PlotWidgetViewComponent implements OnInit, OnDestroy {
  itemId: string;
  systemWindowId: string;

  mouseMove$: Observable<MouseEvent>;
  destroy$ = new Subject<{}>();

  leftOffset = 0;
  topOffset = 0;
  height: number;
  width: number;

  plotWidget: PlotWidget;
  
  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
      this.plotWidget=new PlotWidget(this.electronService.minsky.namedItems.elem(this.itemId).second);
    });
  }

  ngOnInit() {
    setTimeout(()=>{this.render();},10);

    const minskyCanvasElement = document.getElementById('plot-cairo-canvas') as HTMLElement;
    this.mouseMove$ = fromEvent<MouseEvent>(
      minskyCanvasElement,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    this.mouseMove$.pipe(takeUntil(this.destroy$)).subscribe(async (event: MouseEvent) => {
      await this.plotWidget.mouseMove(event.x,event.y);
    });
    
    minskyCanvasElement.addEventListener(
      'mousedown',
      async (event: MouseEvent) => {
        await this.plotWidget.mouseDown(event.x,event.y);
      }
    );

    minskyCanvasElement.addEventListener(
      'mouseup',
      async (event: MouseEvent) => {
        await this.plotWidget.mouseUp(event.x,event.y);
      }
    );
  }

  async render() {
    const plotCanvasContainer = document.getElementById('plot-cairo-canvas');

    const clientRect = plotCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);

    this.topOffset = Math.round(
      await this.windowUtilityService.getElectronMenuBarHeight()
    );

    this.height = Math.round(plotCanvasContainer.clientHeight);
    this.width = Math.round(plotCanvasContainer.clientWidth);

    if (
      this.electronService.isElectron &&
      this.systemWindowId &&
      this.itemId &&
      this.height &&
      this.width
    ) {
      this.plotWidget.renderFrame({
          parentWindowId: this.systemWindowId.toString(),
          offsetLeft: this.leftOffset,
          offsetTop: this.topOffset,
          childWidth: this.width,
          childHeight: this.height,
          scalingFactor: -1
        });
    }
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
