import {
  ChangeDetectorRef,
  Component,
  NgZone,
  OnDestroy,
  OnInit,
} from '@angular/core';
import {
  CommunicationService,
  ElectronService,
  WindowUtilityService,
} from '@minsky/core';
import { commandsMapping, MainRenderingTabs } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { fromEvent, Observable } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-wiring',
  templateUrl: './wiring.component.html',
  styleUrls: ['./wiring.component.scss'],
})
export class WiringComponent implements OnInit, OnDestroy {
  mouseMove$: Observable<MouseEvent>;
  canvasContainerHeight: string;
  availableOperationsMapping: Record<string, string[]> = {};
  showDragCursor = false;
  wiringTab = MainRenderingTabs.canvas;
  constructor(
    public cmService: CommunicationService,
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService,
    private zone: NgZone,
    private changeDetectorRef: ChangeDetectorRef
  ) {}

  async ngOnInit() {
    const isMainWindow = this.windowUtilityService.isMainWindow();
    if (isMainWindow) {
      const minskyCanvasContainer = this.windowUtilityService.getMinskyContainerElement();
      this.cmService.showDragCursor$.subscribe((showDragCursor) => {
        this.showDragCursor = showDragCursor;
        this.changeDetectorRef.detectChanges();
      });

      this.setupEventListenersForCanvas(minskyCanvasContainer);
    }

    this.availableOperationsMapping = (await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.AVAILABLE_OPERATIONS_MAPPING,
        render: false,
      }
    )) as Record<string, string[]>;

    setTimeout(async () => {
      await this.electronService.sendMinskyCommandAndRender({
        command: commandsMapping.REQUEST_REDRAW_SUBCOMMAND,
      });
    }, 1);
  }

  private setupEventListenersForCanvas(minskyCanvasContainer: HTMLElement) {
    const minskyCanvasElement = this.windowUtilityService.getMinskyCanvasElement();
    const scrollableArea = this.windowUtilityService.getScrollableArea();

    this.zone.runOutsideAngular(() => {
      if (this.electronService.isElectron) {
        const handleScroll = async (scrollTop: number, scrollLeft: number) => {
          const posX = scrollableArea.width / 2 - scrollLeft;
          const posY = scrollableArea.height / 2 - scrollTop;
          await this.electronService.sendMinskyCommandAndRender({
            command: commandsMapping.MOVE_TO,
            mouseX: posX,
            mouseY: posY,
          });
        };

        minskyCanvasContainer.addEventListener('scroll', async () => {
          // TODO: handle scroll for different tabs
          await handleScroll(
            minskyCanvasContainer.scrollTop,
            minskyCanvasContainer.scrollLeft
          );
        });
        minskyCanvasContainer.onwheel = this.cmService.onMouseWheelZoom;

        // TextInputUtilities.bindEvents();

        document.body.addEventListener('keydown', async (event) => {
          // TextInputUtilities.show();
          await this.cmService.handleKeyDown({ event });
        });

        document.body.addEventListener('keyup', async (event) => {
          await this.cmService.handleKeyUp(event);
        });

        this.mouseMove$ = fromEvent<MouseEvent>(
          minskyCanvasElement,
          'mousemove'
        ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

        this.mouseMove$.subscribe(async (event: MouseEvent) => {
          await this.cmService.mouseEvents('CANVAS_EVENT', event);
        });

        minskyCanvasElement.addEventListener(
          'mousedown',
          async (event: MouseEvent) => {
            await this.cmService.mouseEvents('CANVAS_EVENT', event);
          }
        );

        minskyCanvasElement.addEventListener(
          'contextmenu',
          async (event: MouseEvent) => {
            await this.cmService.mouseEvents('contextmenu', event);
          }
        );

        minskyCanvasElement.addEventListener(
          'mouseup',
          async (event: MouseEvent) => {
            await this.cmService.mouseEvents('CANVAS_EVENT', event);
          }
        );

        minskyCanvasElement.addEventListener('dblclick', () => {
          if (this.cmService.currentTab === MainRenderingTabs.canvas) {
            this.cmService.handleDblClick();
          }
        });
      }
    });
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
