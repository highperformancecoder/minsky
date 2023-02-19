import { Component, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { PlotWidget } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-plot-widget-view',
  templateUrl: './plot-widget-view.component.html',
  styleUrls: ['./plot-widget-view.component.scss'],
})
export class PlotWidgetViewComponent implements OnInit, OnDestroy {
  itemId: string;
  systemWindowId: string;

  leftOffset = 0;
  topOffset = 0;
  height: number;
  width: number;

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  ngOnInit() {
    setTimeout(()=>{this.render();},10);
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
      new PlotWidget(this.electronService.minsky.namedItems.elem(this.itemId).second)
        .renderFrame({
          parentWindowId: this.systemWindowId.toString(),
          offsetLeft: this.leftOffset,
          offsetTop: this.topOffset,
          childWidth: this.width,
          childHeight: this.height,
          scalingFactor: -1
        });
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
