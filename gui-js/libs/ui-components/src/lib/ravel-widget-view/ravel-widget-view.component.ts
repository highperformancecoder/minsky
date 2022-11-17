import { Component, OnDestroy, ElementRef, AfterViewInit, ViewChild,} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {   CommunicationService, ElectronService, WindowUtilityService } from '@minsky/core';
import { events, Ravel, isMacOS } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { fromEvent, Observable } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-ravel-widget-view',
  templateUrl: './ravel-widget-view.component.html',
  styleUrls: ['./ravel-widget-view.component.scss'],
})
export class RavelViewComponent implements AfterViewInit, OnDestroy {
  @ViewChild('ravelCanvasWrapper') ravelCanvasWrapper: ElementRef;
  itemId: string;
  systemWindowId: string;

  leftOffset = 0;
  topOffset = 0;
  height: number;
  width: number;
  namedItem: Ravel;
  mouseMove$: Observable<MouseEvent>;
  yoffs = 0; // extra offset required on some systems
  context=false;
  
  constructor(
    private communicationService: CommunicationService,
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  ngAfterViewInit() {
    this.namedItem=new Ravel(this.electronService.minsky.namedItems.elem(this.itemId).second);
    this.render();
    this.initEvents();
    if (isMacOS()) this.yoffs=-20; // why, o why, Mac?
  }

  render() {
    const ravelCanvasContainer = this.ravelCanvasWrapper.nativeElement as HTMLElement;

    const clientRect = ravelCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);

    this.topOffset = Math.round(
      this.windowUtilityService.getElectronMenuBarHeight()
    );

    this.height = Math.round(ravelCanvasContainer.clientHeight);
    this.width = Math.round(ravelCanvasContainer.clientWidth);

    if (
      this.electronService.isElectron &&
      this.systemWindowId &&
      this.itemId &&
      this.height &&
      this.width
    ) {
      this.namedItem.popup
        .renderFrame(this.systemWindowId,this.leftOffset,this.topOffset,this.width,this.height,-1);
    }
  }

  initEvents() {
    const ravelCanvasContainer = this.ravelCanvasWrapper.nativeElement as HTMLElement;

    this.mouseMove$ = fromEvent<MouseEvent>(
      ravelCanvasContainer,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    this.mouseMove$.subscribe(async (event: MouseEvent) => {
      if (event.buttons==0)
        this.namedItem.popup.mouseOver(event.x,event.y+this.yoffs);
      else
        this.namedItem.popup.mouseMove(event.x,event.y+this.yoffs);
    });

    ravelCanvasContainer.addEventListener('mousedown', async (event) => {
      this.namedItem.popup.mouseDown(event.x,event.y+this.yoffs);
    });

    ravelCanvasContainer.addEventListener('mouseup', async (event) => {
      this.namedItem.popup.mouseUp(event.x,event.y+this.yoffs);
    });
    ravelCanvasContainer.addEventListener('mouseleave', async (event) => {
      if (!this.context) // absorb mouseleaves caused by context menu posting
        this.namedItem.popup.mouseLeave();
      this.context=false;
    });

    ravelCanvasContainer.addEventListener('contextmenu', async (event) => {
      this.context=true;
      this.electronService.ipcRenderer.send(events.CONTEXT_MENU, {
        x: event.x,
        y: event.y,
        type: "ravel",
        command: this.namedItem.prefix(),
      });
    });

    document.onkeydown = async (event) => {
      process.stdout.write(`keydown: ${event.key}\n`)
      this.communicationService.mouseX=this.communicationService.mouseY=0;
      await this.communicationService.handleKeyDown({
        event,
        command: `/minsky/namedItems/@elem/"${this.itemId}"/second/popup`,
      });
    };
  }
  
   // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
