import {
  AfterViewInit,
  Component,
  ElementRef,
  OnDestroy,
  ViewChild,
} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {
  CommunicationService,
  ElectronService,
  WindowUtilityService,
} from '@minsky/core';
import {
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR,
  events,
  GodleyIcon,
  GodleyTableWindow,
  isMacOS,
  green
} from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { fromEvent, Observable } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-godley-widget-view',
  templateUrl: './godley-widget-view.component.html',
  styleUrls: ['./godley-widget-view.component.scss'],
})
export class GodleyWidgetViewComponent implements OnDestroy, AfterViewInit {
  @ViewChild('godleyCanvasElemWrapper') godleyCanvasElemWrapper: ElementRef;

  itemId: string;
  systemWindowId: string;
  namedItem: GodleyIcon;
  namedItemSubCommand: GodleyTableWindow;

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  godleyCanvasContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;

  mouseX = 0;
  mouseY = 0;
  yoffs = 0; // extra offset required on some systems

  constructor(
    private communicationService: CommunicationService,
    private windowUtilityService: WindowUtilityService,
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  ngAfterViewInit() {
    this.namedItem = new GodleyIcon(this.electronService.minsky.namedItems.elem(this.itemId).second);
    this.namedItemSubCommand = this.namedItem.popup;
    this.getWindowRectInfo();
    this.renderFrame();
    this.initEvents();
    if (isMacOS()) this.yoffs=-20; // why, o why, Mac?
  }

  windowResize() {
    this.getWindowRectInfo();
    this.renderFrame();
  }
  private getWindowRectInfo() {
    this.godleyCanvasContainer = this.godleyCanvasElemWrapper
      .nativeElement as HTMLElement;

    const clientRect = this.godleyCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);
    this.topOffset = Math.round(
      this.windowUtilityService.getElectronMenuBarHeight()
    );

    this.height = Math.round(this.godleyCanvasContainer.clientHeight);
    this.width = Math.round(this.godleyCanvasContainer.clientWidth);
  }

  renderFrame() {
    if (
      this.electronService.isElectron &&
      this.systemWindowId &&
      this.itemId &&
      this.height &&
      this.width
    ) {
      const scaleFactor = this.electronService.remote.screen.getPrimaryDisplay()
        .scaleFactor;

      this.namedItemSubCommand.renderFrame(this.systemWindowId,this.leftOffset,this.topOffset,this.width,this.height,scaleFactor);
    }
  }

  initEvents() {
    this.godleyCanvasContainer.addEventListener('scroll', async () => {
      await this.handleScroll(
        this.godleyCanvasContainer.scrollTop,
        this.godleyCanvasContainer.scrollLeft
      );
    });

    this.mouseMove$ = fromEvent<MouseEvent>(
      this.godleyCanvasContainer,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    this.mouseMove$.subscribe(async (event: MouseEvent) => {
      this.namedItemSubCommand.mouseMove(event.x,event.y+this.yoffs);
    });

    this.godleyCanvasContainer.addEventListener('mousedown', async (event) => {
      this.electronService.ipcRenderer.invoke(events.GODLEY_VIEW_MOUSEDOWN, {
        command: `/minsky/namedItems/@elem/"${this.itemId}"/second/popup`,
        mouseX: event.x, mouseY: event.y+this.yoffs
      });
    });

    this.godleyCanvasContainer.addEventListener('mouseup', async (event) => {
      this.namedItemSubCommand.mouseDown(event.x,event.y+this.yoffs);
    });
    
    this.godleyCanvasContainer.addEventListener('contextmenu', async (event) => {
      this.electronService.ipcRenderer.send(events.CONTEXT_MENU, {
        x: this.mouseX,
        y: this.mouseY,
        type: "godley",
        command: this.namedItem,
      });
    });

    this.godleyCanvasContainer.onwheel = this.onMouseWheelZoom;
    document.onkeydown = this.onKeyDown;
  }

  async redraw() {
    this.namedItemSubCommand.requestRedraw();
  }

  onKeyDown = async (event: KeyboardEvent) => {
    await this.communicationService.handleKeyDown({
      event,
      command: `/minsky/namedItems/@elem/"${this.itemId}"/second/popup`,
    });

    await this.redraw();
  };

  onMouseWheelZoom = async (event: WheelEvent) => {
    event.preventDefault();
    const zoomFactor = event.deltaY<0 ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;

    const [
      x,
      y,
    ] = this.electronService.remote.getCurrentWindow().getContentSize();

    //TODO: throttle here if required
    this.namedItemSubCommand.zoom(x/2, y/2, zoomFactor);
  };

  async handleScroll(scrollTop: number, scrollLeft: number) {
    //TODO: throttle here if required

    const cols = await this.namedItem.table.cols();
    const rows = await this.namedItem.table.rows();

    const stepX = this.godleyCanvasContainer.scrollHeight / (cols-1);
    const stepY = this.godleyCanvasContainer.scrollHeight / (rows-1);

    const currentStepX = Math.round(scrollLeft / stepX)+1;
    const currentStepY = Math.round(scrollTop / stepY)+1;

    this.namedItemSubCommand.scrollColStart(currentStepX);
    this.namedItemSubCommand.scrollRowStart(currentStepY);
    this.redraw();
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
