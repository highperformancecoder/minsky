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
import { events, MainRenderingTabs, minsky } from '@minsky/shared';
import { fromEvent, Observable, Subject, takeUntil } from 'rxjs';
import { sampleTime } from 'rxjs/operators';
import { AvailableOperationsComponent } from './available-operations/available-operations.component';
import { VariableComponent } from './variable/variable.component';
import { NgIf, NgFor, KeyValuePipe } from '@angular/common';

@Component({
    selector: 'minsky-wiring',
    templateUrl: './wiring.component.html',
    styleUrls: ['./wiring.component.scss'],
    standalone: true,
    imports: [
        NgIf,
        VariableComponent,
        NgFor,
        AvailableOperationsComponent,
        KeyValuePipe,
    ],
})
export class WiringComponent implements OnInit, OnDestroy {
  mouseMove$: Observable<MouseEvent>;
  canvasContainerHeight: string;
  availableOperationsMapping: any; //Record<string, string[]>;
  wiringTab = MainRenderingTabs.canvas;

  destroy$ = new Subject<{}>();

  constructor(
    public cmService: CommunicationService,
    public electronService: ElectronService,
    private windowUtilityService: WindowUtilityService,
    private zone: NgZone,
    public changeDetectorRef: ChangeDetectorRef
  ) {}

  async ngOnInit() {
    const isMainWindow = await this.windowUtilityService.isMainWindow();
    if (isMainWindow)
      setTimeout(async ()=> 
        {
          await this.windowUtilityService.reInitialize();
          const minskyCanvasContainer = this.windowUtilityService.getMinskyContainerElement();
          this.setupEventListenersForCanvas(minskyCanvasContainer);
        }, 10);

    this.availableOperationsMapping = await this.electronService.minsky.availableOperationsMapping();

    setTimeout(async () => {minsky.canvas.requestRedraw();}, 1);
  }

  private setupEventListenersForCanvas(minskyCanvasContainer: HTMLElement) {
    const minskyCanvasElement = document.getElementById('main-minsky-canvas') as HTMLElement;
    const scrollableArea = this.windowUtilityService.getScrollableArea();
    var moveOnScroll=0; // avoid moving when programmatically setting scroll bars

    const tabElements = document.getElementsByClassName('tab');
    for(let i = 0; i < tabElements.length; i++) {
      const te = <HTMLElement>tabElements.item(i);
      te.addEventListener('dragstart', event => {
        te.click();
        event.stopPropagation();
        event.preventDefault();
      });
    }

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
        minskyCanvasContainer.addEventListener('RESET_SCROLL', async () => {
          this.cmService.resetScroll();
        });
        minskyCanvasContainer.onwheel = this.cmService.onMouseWheelZoom;

        document.body.onkeydown=async (event) => {
          if (event.key==='Shift' && document?.body?.style)
            document.body.style.cursor='grab';
          // check if over a toolbar button. We have to do it in such a kludgy way, because keydown events are only sent to focussed elements, and typically buttons do not obtain focus.
          if (event.code==='F1') {
            let elems=document.querySelectorAll(':hover');
            var elem;
            for (let i=0; i<elems.length; ++i)
              if (elems[i].classList.contains('toolButton')) {
                elem=elems[i];
                break;
              }
            let classType=elem?.getAttribute('classType');
            if (!classType && elem) classType=elem['classType']; // submenu items render custom attributes differently
            if (classType) {
              this.electronService.send(events.HELP_FOR, {classType});
              return;
            }
          }
          // TODO - move this method out of cmService
          await this.cmService.handleKeyDown({ event });
        };

        document.body.onkeyup=async (event) => {
          if (event.key==='Shift' && document?.body?.style)
            document.body.style.cursor='default';          
          await this.cmService.handleKeyUp(event);
        };

        this.electronService.on(events.CURSOR_BUSY, async (event, busy: boolean)=>{
          document.body.style.cursor=busy? 'wait': 'default';
        });
        
        this.mouseMove$ = fromEvent<MouseEvent>(
          minskyCanvasElement,
          'mousemove'
        ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

        this.mouseMove$.pipe(takeUntil(this.destroy$)).subscribe(async (event: MouseEvent) => {
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

  getKey(operation) {
    return operation.key;
  }

  getValue(operation) {
    return operation.value;
  }

  async addRavel() {
    if (!await this.electronService.minsky.ravelAvailable()) {
      this.electronService.send(events.HELP_FOR, {classType:"Ravel"});
    }
    this.electronService.minsky.canvas.addRavel();
  }
  
  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
