import { Injectable } from '@angular/core';
import { delayBeforeClosingPopupWindow, ElectronCanvasOffset, electronMenuBarHeightForWindows, isWindows } from '@minsky/shared';
import { ElectronService } from '../electron/electron.service';

@Injectable({
  providedIn: 'root',
})
export class WindowUtilityService {
  private minskyCanvasElement: HTMLCanvasElement = null;
  private minskyCanvasContainer: HTMLElement = null;
  private leftOffset = 0;
  private topOffset = 0;
  private electronMenuBarHeight = 0;
  private drawableWidth = 0;
  private drawableHeight = 0;
  private scrollableAreaWidth = null;
  private scrollableAreaHeight = null;
  private mainWindowId = 1;
  SCROLLABLE_AREA_FACTOR = 10;

  constructor(private electronService: ElectronService) { }

  private initializeIfNeeded() {
    if (!this.minskyCanvasElement) {
      this.reInitialize();
    }
  }

  public reInitialize() {
    this.minskyCanvasContainer = document.getElementById(
      'minsky-canvas-container'
    );

    if (
      this.minskyCanvasContainer &&
      this.electronService.remote.getCurrentWindow().id === this.mainWindowId
    ) {
      const bodyElement = document.getElementsByTagName('body')[0];
      this.minskyCanvasElement = document.getElementById(
        'main-minsky-canvas'
      ) as HTMLCanvasElement;

      this.minskyCanvasContainer.style.height =
        bodyElement.clientHeight - this.minskyCanvasContainer.offsetTop + 'px';

      if (!this.scrollableAreaWidth || !this.scrollableAreaHeight) {
        // Scrollable area size should not change

        this.scrollableAreaWidth =
          bodyElement.clientWidth * this.SCROLLABLE_AREA_FACTOR;
        this.scrollableAreaHeight =
          bodyElement.clientHeight * this.SCROLLABLE_AREA_FACTOR;

        // No need to set canvas width / height - we don't use the frontend canvas at all
        // this.minskyCanvasElement.width = this.scrollableAreaWidth;
        // this.minskyCanvasElement.height = this.scrollableAreaHeight;

        this.minskyCanvasElement.style.width = this.scrollableAreaWidth + 'px';
        this.minskyCanvasElement.style.height =
          this.scrollableAreaHeight + 'px';
      }

      // After setting the above, container gets scrollbars, so we need to compute drawableWidth & Height only now (clientWidth/clientHeight change after scrollbar addition)

      this.drawableWidth = this.minskyCanvasContainer.clientWidth;
      this.drawableHeight = this.minskyCanvasContainer.clientHeight;

      const clientRect = this.minskyCanvasContainer.getBoundingClientRect();

      this.leftOffset = clientRect.left;
      this.topOffset = clientRect.top;

      this.electronMenuBarHeight = this.getElectronMenuBarHeight();
    }
  }

  public getElectronMenuBarHeight() {
    const currentWindow = this.electronService.remote.getCurrentWindow();
    const currentWindowSize = currentWindow.getSize()[1];

    const currentWindowContentSize = currentWindow.getContentSize()[1];

    const electronMenuBarHeight = currentWindowSize - currentWindowContentSize;

    const scaleFactor = this.electronService.remote.screen.getPrimaryDisplay()
      .scaleFactor;

    // const windowsMenuBarHeightDifference = 47;
    //https://github.com/electron/electron/issues/17580
    //https://github.com/electron/electron/issues/4045

    return isWindows()
      ? electronMenuBarHeightForWindows
      : electronMenuBarHeight * scaleFactor;
  }

  public scrollToCenter() {
    this.initializeIfNeeded();
    this.minskyCanvasElement.scrollTop = this.scrollableAreaHeight / 2;
    this.minskyCanvasElement.scrollLeft = this.scrollableAreaWidth / 2;
  }

  public getMinskyCanvasOffset(): ElectronCanvasOffset {
    this.initializeIfNeeded();
    return {
      left: this.leftOffset,
      top: this.topOffset,
      electronMenuBarHeight: this.electronMenuBarHeight,
    };
  }

  public getDrawableArea() {
    this.initializeIfNeeded();
    return {
      width: this.drawableWidth,
      height: this.drawableHeight,
    };
  }

  public getScrollableArea() {
    this.initializeIfNeeded();
    return {
      width: this.scrollableAreaWidth,
      height: this.scrollableAreaHeight,
    };
  }

  public getMinskyCanvasElement(): HTMLElement {
    this.initializeIfNeeded();
    return this.minskyCanvasElement;
  }

  public getMinskyContainerElement(): HTMLElement {
    this.initializeIfNeeded();
    return this.minskyCanvasContainer;
  }

  public isMainWindow(): boolean {
    const isMainWindow =
      this.electronService.remote.getCurrentWindow().id === 1;

    return isMainWindow;
  }

  public closeCurrentWindowIfNotMain() {
    const currentWindow = this.electronService.remote.getCurrentWindow();
    console.log(`Scheduling closing of window id=${currentWindow?.id}`);
    console.log(
      '🚀 ~ file: window-utility.service.ts ~ line 147 ~ WindowUtilityService ~ closeCurrentWindowIfNotMain ~ closeCurrentWindowIfNotMain'
    );
    if (this.electronService.isElectron) {
      setTimeout(() => {
        const currentWindow = this.electronService.remote.getCurrentWindow();
        if (currentWindow?.id !== 1) {
          console.log(`Closing window id=${currentWindow?.id}`);
          console.log(currentWindow);
          currentWindow.close();
        }
      }, delayBeforeClosingPopupWindow);
    }
  }
}
