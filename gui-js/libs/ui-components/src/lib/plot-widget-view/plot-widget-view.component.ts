import { Component, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-plot-widget-view',
  templateUrl: './plot-widget-view.component.html',
  styleUrls: ['./plot-widget-view.component.scss'],
})
export class PlotWidgetViewComponent implements OnInit, OnDestroy {
  itemId: number;
  systemWindowId: number;

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
    this.render();
  }

  render() {
    const plotCanvasContainer = document.getElementById('plot-cairo-canvas');

    const clientRect = plotCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);

    this.topOffset = Math.round(
      this.windowUtilityService.getElectronMenuBarHeight()
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
      const scaleFactor = this.electronService.remote.screen.getPrimaryDisplay()
        .scaleFactor;
      const command = `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/renderFrame [${this.systemWindowId},${this.leftOffset},${this.topOffset},${this.width},${this.height},${scaleFactor}]`;

      this.electronService.sendMinskyCommandAndRender({
        command,
      });
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
