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
  commandsMapping,
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR,
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

  itemId: number;
  systemWindowId: number;
  namedItemSubCommand: string;

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  godleyCanvasContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;

  mouseX = 0;
  mouseY = 0;

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
    this.namedItemSubCommand = `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/popup`;
    this.getWindowRectInfo();
    this.renderFrame();
    this.initEvents();
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

      const command = `${this.namedItemSubCommand}/renderFrame [${this.systemWindowId},${this.leftOffset},${this.topOffset},${this.width},${this.height},${scaleFactor}]`;

      this.electronService.sendMinskyCommandAndRender({
        command,
      });
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
      const { clientX, clientY } = event;
      this.mouseX = clientX;
      this.mouseY = clientY;
      this.sendMouseEvent(
        clientX,
        clientY,
        commandsMapping.MOUSEMOVE_SUBCOMMAND
      );
    });

    this.godleyCanvasContainer.addEventListener('mousedown', (event) => {
      const { clientX, clientY } = event;
      this.sendMouseEvent(
        clientX,
        clientY,
        commandsMapping.MOUSEDOWN_SUBCOMMAND
      );
    });

    this.godleyCanvasContainer.addEventListener('mouseup', async (event) => {
      const { clientX, clientY } = event;
      await this.sendMouseEvent(
        clientX,
        clientY,
        commandsMapping.MOUSEUP_SUBCOMMAND
      );
    });

    this.godleyCanvasContainer.onwheel = this.onMouseWheelZoom;
    document.onkeydown = this.onKeyDown;
  }

  async redraw() {
    await this.electronService.sendMinskyCommandAndRender({
      command: `${this.namedItemSubCommand}/requestRedraw`,
    });
  }

  async sendMouseEvent(x: number, y: number, type: string) {
    const command = `${this.namedItemSubCommand}/${type} [${x},${y}]`;

    await this.electronService.sendMinskyCommandAndRender({
      command,
    });

    await this.redraw();
  }

  onKeyDown = async (event: KeyboardEvent) => {
    await this.communicationService.handleKeyDown({
      event,
      command: this.namedItemSubCommand,
    });

    await this.redraw();
  };

  onMouseWheelZoom = async (event: WheelEvent) => {
    event.preventDefault();
    const { deltaY } = event;
    const zoomIn = deltaY < 0;

    const command = `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/popup/zoom`;

    const zoomFactor = zoomIn ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;

    const [
      x,
      y,
    ] = this.electronService.remote.getCurrentWindow().getContentSize();

    //TODO: throttle here if required
    await this.electronService.sendMinskyCommandAndRender({
      command: `${command} [${x / 2},${y / 2},${zoomFactor}]`,
    });
  };

  async handleScroll(scrollTop: number, scrollLeft: number) {
    //TODO: throttle here if required

    const cols = (await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/table/cols`,
    })) as number;

    const rows = (await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.GET_NAMED_ITEM}/${this.itemId}/second/table/rows`,
    })) as number;

    const stepX = this.godleyCanvasContainer.scrollHeight / cols;
    const stepY = this.godleyCanvasContainer.scrollHeight / rows;

    const currentStepX = Math.round(scrollLeft / stepX);
    const currentStepY = Math.round(scrollTop / stepY);

    await this.electronService.sendMinskyCommandAndRender({
      command: `${this.namedItemSubCommand}/scrollColStart ${currentStepX}`,
    });

    await this.electronService.sendMinskyCommandAndRender({
      command: `${this.namedItemSubCommand}/scrollRowStart ${currentStepY}`,
    });

    this.redraw();
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
