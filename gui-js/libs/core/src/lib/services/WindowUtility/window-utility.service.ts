import { Injectable } from '@angular/core';
import { delayBeforeClosingPopupWindow, ElectronCanvasOffset,} from '@minsky/shared';
import { ElectronService } from '../electron/electron.service';

@Injectable({
  providedIn: 'root',
})
export class WindowUtilityService {
  private minskyCanvasElement: HTMLCanvasElement = null;
  private minskyCanvasContainer: HTMLElement = null;
  private leftOffset = 0;
  private topOffset = 0;
  private drawableWidth = 0;
  private drawableHeight = 0;
  private scrollableAreaWidth = null;
  private scrollableAreaHeight = null;
  private mainWindowId = 1;
  // scrollable area is this multiple of the extent of the canvas contents
  SCROLLABLE_AREA_FACTOR = 1.5;

  constructor(private electronService: ElectronService) { }

  public async reInitialize() {
    this.minskyCanvasContainer = document.getElementById(
      'minsky-canvas-container'
    );

    let currentWindow=await this.electronService.getCurrentWindow();
    if (this.minskyCanvasContainer && currentWindow.id === this.mainWindowId) {
      const bodyElement = document.getElementsByTagName('body')[0];
      this.minskyCanvasElement = document.getElementById(
        'main-minsky-canvas'
      ) as HTMLCanvasElement;

      this.minskyCanvasContainer.style.height =
        bodyElement.clientHeight - this.minskyCanvasContainer.offsetTop + 'px';

      if (!this.scrollableAreaWidth || !this.scrollableAreaHeight) {
        // initial size, until updateScrollableArea is called with the contents' bounds
        this.scrollableAreaWidth = bodyElement.clientWidth * 3;
        this.scrollableAreaHeight = bodyElement.clientHeight * 3;
      }
      this.minskyCanvasElement.style.width = this.scrollableAreaWidth + 'px';
      this.minskyCanvasElement.style.height = this.scrollableAreaHeight + 'px';

      // After setting the above, container gets scrollbars, so we need to compute drawableWidth & Height only now (clientWidth/clientHeight change after scrollbar addition)

      const clientRect = this.minskyCanvasContainer.getBoundingClientRect();
      const documentRect = document.documentElement.getBoundingClientRect();

      this.leftOffset = clientRect.left;
      this.topOffset = clientRect.top-documentRect.top;
      this.drawableWidth = clientRect.right-clientRect.left;
      this.drawableHeight = clientRect.bottom-clientRect.top;
    }
  }

  /**
   * Resize the scrollable area to SCROLLABLE_AREA_FACTOR times the extent of
   * the canvas contents. The pan position (scrollable area centre minus
   * scroll offset) lies within +/- half the excess over the drawable area.
   * @param bounds [x0,y0,x1,y1] of canvas contents in window coordinates
   * @param pos current pan position [x,y]
   * @returns true if the scrollable area changed
   */
  public updateScrollableArea(bounds: number[], pos: number[]): boolean {
    if (!this.minskyCanvasElement || !bounds || bounds.length < 4) return false;
    const size = (b0: number, b1: number, p: number, drawable: number) => {
      // contents' extent relative to the unpanned origin
      const extent = Math.max(Math.abs(b0 - p), Math.abs(b1 - p), 0);
      const half = this.SCROLLABLE_AREA_FACTOR * extent;
      return Math.ceil(Math.max(drawable + 2 * half, 2 * drawable));
    };
    const w = size(bounds[0], bounds[2], pos[0], this.drawableWidth);
    const h = size(bounds[1], bounds[3], pos[1], this.drawableHeight);
    if (w === this.scrollableAreaWidth && h === this.scrollableAreaHeight) return false;
    this.scrollableAreaWidth = w;
    this.scrollableAreaHeight = h;
    this.minskyCanvasElement.style.width = w + 'px';
    this.minskyCanvasElement.style.height = h + 'px';
    return true;
  }

  public scrollToCenter() {
    this.minskyCanvasElement.scrollTop = this.scrollableAreaHeight / 2;
    this.minskyCanvasElement.scrollLeft = this.scrollableAreaWidth / 2;
  }

  public getMinskyCanvasOffset(): ElectronCanvasOffset {
    return {
      left: this.leftOffset,
      top: this.topOffset,
    };
  }

  public getDrawableArea() {
    return {
      width: this.drawableWidth,
      height: this.drawableHeight,
    };
  }

  public getScrollableArea() {
    return {
      width: this.scrollableAreaWidth,
      height: this.scrollableAreaHeight,
    };
  }

  public getMinskyCanvasElement(): HTMLElement {
    return this.minskyCanvasElement;
  }

  public getMinskyContainerElement(): HTMLElement {
    return this.minskyCanvasContainer;
  }

  public async isMainWindow(): Promise<boolean> {
    let currentWindow=await this.electronService.getCurrentWindow();
    return currentWindow.id === 1;
  }

  public closeCurrentWindowIfNotMain() {
    if (this.electronService.isElectron) {
      setTimeout(async () => {
        const currentWindow = await this.electronService.getCurrentWindow();
        if (currentWindow?.id !== 1) {
          this.electronService.closeWindow();
        }
      }, delayBeforeClosingPopupWindow);
    }
  }
}
