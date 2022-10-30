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
import { MainRenderingTabs, minsky } from '@minsky/shared';
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
  availableOperationsMapping: any; //Record<string, string[]>;
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

    this.availableOperationsMapping = await this.electronService.minsky.availableOperationsMapping();

    setTimeout(async () => {minsky.canvas.requestRedraw();}, 1);
  }

  private setupEventListenersForCanvas(minskyCanvasContainer: HTMLElement) {
    const minskyCanvasElement = this.windowUtilityService.getMinskyCanvasElement();
    const scrollableArea = this.windowUtilityService.getScrollableArea();
    var moveOnScroll=0; // avoid moving when programmatically setting scroll bars
    
    this.cmService.resetScroll=async ()=>{
      ++moveOnScroll;
      var pos= await this.electronService.currentTabPosition();
      minskyCanvasContainer.scrollLeft=scrollableArea.width/2 - pos[0];
      minskyCanvasContainer.scrollTop=scrollableArea.height/2 - pos[1];
    };
    this.cmService.resetScroll();
    
   this.zone.runOutsideAngular(() => {
      if (this.electronService.isElectron) {
        const handleScroll = async (scrollTop: number, scrollLeft: number) => {
          const posX = scrollableArea.width / 2 - scrollLeft;
          const posY = scrollableArea.height / 2 - scrollTop;
          if (!moveOnScroll)
            await this.electronService.currentTabMoveTo(posX,posY);
          else
            --moveOnScroll;
        };

        minskyCanvasContainer.addEventListener('scroll', async () => {
          await handleScroll(
            minskyCanvasContainer.scrollTop,
            minskyCanvasContainer.scrollLeft
          );
        });
        minskyCanvasContainer.addEventListener('RESET_ZOOM', async () => {
          minskyCanvasContainer.scrollTop=scrollableArea.width / 2;
          minskyCanvasContainer.scrollLeft=scrollableArea.height / 2;
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
