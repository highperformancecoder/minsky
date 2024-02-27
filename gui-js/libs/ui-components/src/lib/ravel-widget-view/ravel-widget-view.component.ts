import { Component, OnDestroy, ElementRef, AfterViewInit, ViewChild,} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {   CommunicationService, ElectronService, WindowUtilityService } from '@minsky/core';
import { events, Ravel, Functions } from '@minsky/shared';
import { fromEvent, Observable, Subject, takeUntil } from 'rxjs';
import { sampleTime } from 'rxjs/operators';
import { MessageBoxSyncOptions } from 'electron/renderer';

@Component({
  selector: 'minsky-ravel-widget-view',
  templateUrl: './ravel-widget-view.component.html',
  styleUrls: ['./ravel-widget-view.component.scss'],
  standalone: true
})
export class RavelViewComponent implements AfterViewInit, OnDestroy {
  @ViewChild('ravelCanvasWrapper') ravelCanvasWrapper: ElementRef;
  itemId: string;
  systemWindowId: string;

  destroy$ = new Subject<{}>();

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
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  ngAfterViewInit() {
    if(!this.electronService.isElectron) return;

    this.namedItem=new Ravel(this.electronService.minsky.namedItems.elem(this.itemId).second);
    this.render();
    this.initEvents();
    if (Functions.isMacOS()) this.yoffs=-20; // why, o why, Mac?
  }

  async render() {
    const ravelCanvasContainer = this.ravelCanvasWrapper.nativeElement as HTMLElement;

    const clientRect = ravelCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);

    this.topOffset = Math.round(
      await this.windowUtilityService.getElectronMenuBarHeight()
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
      try {
        this.namedItem.popup
          .renderFrame({
            parentWindowId: this.systemWindowId.toString(),
            offsetLeft: this.leftOffset,
            offsetTop: this.topOffset,
            childWidth: this.width,
            childHeight: this.height,
            scalingFactor: -1
          });
      } catch(ex) {
        const options: MessageBoxSyncOptions = {
          buttons: ['Yes'],
          message: ex,
          title: '' + this.topOffset,
        };
  
        await this.electronService.showMessageBoxSync(options);
      }
    }
  }

  initEvents() {
    const ravelCanvasContainer = this.ravelCanvasWrapper.nativeElement as HTMLElement;

    this.mouseMove$ = fromEvent<MouseEvent>(
      ravelCanvasContainer,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    this.mouseMove$.pipe(takeUntil(this.destroy$)).subscribe(async (event: MouseEvent) => {
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
      this.electronService.send(events.CONTEXT_MENU, {
        x: event.x,
        y: event.y,
        type: 'ravel',
        command: this.namedItem.$prefix(),
      });
    });

    document.onkeydown = async (event) => {
      process.stdout.write(`keydown: ${event.key}\n`)
      this.communicationService.mouseX=this.communicationService.mouseY=0;
      await this.communicationService.handleKeyDown({
        event,
        command: `minsky.namedItems.@elem."${this.itemId}".second.popup`,
      });
    };
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
