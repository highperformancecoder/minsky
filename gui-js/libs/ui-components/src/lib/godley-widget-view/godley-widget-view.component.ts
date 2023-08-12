import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  ElementRef,
  OnDestroy,
  OnInit,
  ViewChild
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
  GodleyTableWindow
} from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { fromEvent, Observable } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

import * as JSON5 from 'json5';
import { ScaleHandler } from '../scale-handler/scale-handler.class';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-godley-widget-view',
  templateUrl: './godley-widget-view.component.html',
  styleUrls: ['./godley-widget-view.component.scss'],
})
export class GodleyWidgetViewComponent implements OnDestroy, OnInit, AfterViewInit {
  @ViewChild('godleyCanvasElemWrapper') godleyCanvasElemWrapper: ElementRef;

  itemId: string;
  systemWindowId: string;
  windowId: number;
  godleyIcon: GodleyIcon;
  namedItemSubCommand: GodleyTableWindow;

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  godleyCanvasContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;

  Math = Math;

  renderTimeout;

  zoomFactor=1;
  
  // in canvas mode, the C++ layer writes to the godley-cairo-canvas element
  // this can be re-enabled to compare the visual output and interaction between the canvas based renderer and the HTML based one
  // switching this interactively without reinitialzing the window doesn't work, because the C++ output is very persistent. may need to clear it somehow.
  canvasMode = false;

  htmlModeReady = false;

  columnVariables = [];
  flows = [];

  multiEquityAllowed = false;

  initialValues = [];
  rowSums = [];
  cellValues = [];

  scale = new ScaleHandler();

  cellEditing = [undefined, undefined];

  yoffs = 0; // extra offset required on some systems

  constructor(
    private communicationService: CommunicationService,
    private windowUtilityService: WindowUtilityService,
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private cdRef: ChangeDetectorRef
  ) {
    this.route.queryParams.subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  async ngOnInit() {
    this.godleyIcon = new GodleyIcon(this.electronService.minsky.namedItems.elem(this.itemId).second);
    this.namedItemSubCommand = this.godleyIcon.popup;

    this.windowId = (await this.electronService.getCurrentWindow()).id;

    if(!this.canvasMode) {
      this.electronService.on(events.GODLEY_POPUP_REFRESH, async e => {
        await this.hardRefresh();
      });
    this.electronService.on(events.ZOOM, (event, ratio)=>{this.zoom(ratio);});
      this.electronService.on(events.RESET_ZOOM, (event, ratio)=>{
        this.zoomFactor=1;
        document.body.style.setProperty('zoom','100%');
      });

      await this.hardRefresh();
    }
  }

  ngAfterViewInit() {
    if(this.canvasMode) {
      this.windowResize();
      this.initEvents();
      if (this.electronService.isMacOS()) this.yoffs=-20; // why, o why, Mac?
    }
  }

  async windowResize() {
    await this.getWindowRectInfo();

    clearTimeout(this.renderTimeout);
    this.renderTimeout = setTimeout(() => this.renderFrame(), 300);
  }

  private async getWindowRectInfo() {
    this.godleyCanvasContainer = this.godleyCanvasElemWrapper.nativeElement as HTMLElement;

    const clientRect = this.godleyCanvasContainer.getBoundingClientRect();

    let menuBarHeight=await this.windowUtilityService.getElectronMenuBarHeight();
    this.leftOffset = Math.round(clientRect.left);
    this.topOffset = Math.round(menuBarHeight);

    this.height = Math.round(this.godleyCanvasContainer.clientHeight);
    this.width = Math.round(this.godleyCanvasContainer.clientWidth);
  }

  renderFrame() {
    if (
      this.canvasMode &&
      this.systemWindowId &&
      this.itemId &&
      this.height &&
      this.width
    ) {
      this.namedItemSubCommand.renderFrame({
        parentWindowId: this.systemWindowId.toString(),
        offsetLeft: this.leftOffset,
        offsetTop: this.topOffset,
        childWidth: this.width,
        childHeight: this.height,
        scalingFactor: -1
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
      this.namedItemSubCommand.mouseMove(event.x,event.y+this.yoffs);
    });

    this.godleyCanvasContainer.addEventListener('mousedown', async (event) => {
      if (event.button===0)
        this.electronService.invoke(events.GODLEY_VIEW_MOUSEDOWN, {
          command: this.itemId,
          mouseX: event.x, mouseY: event.y+this.yoffs
        });
    });

    this.godleyCanvasContainer.addEventListener('mouseup', async (event) => {
      this.namedItemSubCommand.mouseUp(event.x,event.y+this.yoffs);
    });
    
    this.godleyCanvasContainer.addEventListener('contextmenu', async (event) => {
      this.electronService.send(events.CONTEXT_MENU, {
        x: event.x,
        y: event.y,
        type: 'godley',
        command: this.godleyIcon.$prefix(),
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
      command: `minsky.namedItems.@elem."${this.itemId}".second.popup`,
    });

    await this.redraw();
  };

  onMouseWheelZoom = async (event: WheelEvent) => {
    event.preventDefault();
    const zoomFactor = event.deltaY<0 ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;

    const [
      x,
      y,
    ] = (await this.electronService.getCurrentWindow()).contentSize;

    //TODO: throttle here if required
    this.namedItemSubCommand.zoom(x/2, y/2, zoomFactor);
  };

  async handleScroll(scrollTop: number, scrollLeft: number) {
    //TODO: throttle here if required

    const cols = await this.godleyIcon.table.cols();
    const rows = await this.godleyIcon.table.rows();

    const stepX = this.godleyCanvasContainer.scrollHeight / (cols-1);
    const stepY = this.godleyCanvasContainer.scrollHeight / (rows-1);

    const currentStepX = Math.round(scrollLeft / stepX)+1;
    const currentStepY = Math.round(scrollTop / stepY)+1;

    this.namedItemSubCommand.scrollColStart(currentStepX);
    this.namedItemSubCommand.scrollRowStart(currentStepY);
    this.redraw();
  }

  async contextMenu(i: number, j: number, clickType: string) {
    const frameId = (await this.electronService.getCurrentWindow()).id;

    this.electronService.send(events.CONTEXT_MENU, {
      x: i,
      y: j,
      type: 'html-godley',
      command: JSON5.stringify([this.godleyIcon.$prefix(), clickType, frameId]),
    });
  }

  async onRowAdd(i) {
    await this.finishEditing();
    this.godleyIcon.table.insertRow(i+3);
    await this.hardRefresh();
  }

  async onRowDelete(i) {
    await this.finishEditing();
    this.godleyIcon.deleteRow(i+3);
    await this.hardRefresh();
  }

  async onRowMove(i, n) {
    await this.finishEditing();
    this.godleyIcon.table.moveRow(i+2,n);
    await this.hardRefresh();
  }

  async onColumnAdd(i) {
    await this.finishEditing();
    this.godleyIcon.table.insertCol(i+2);
    await this.hardRefresh();
  }

  async onColumnDelete(i) {
    await this.finishEditing();
    this.godleyIcon.table.deleteCol(i+2);
    await this.hardRefresh();
  }

  async onColumnMove(i, n) {
    await this.finishEditing();
    this.godleyIcon.table.moveCol(i+1,n);
    await this.hardRefresh();
  }

  async hardRefresh(update = true) {
    this.multiEquityAllowed = await this.electronService.minsky.multipleEquities();

    if(update) this.godleyIcon.update();

    const allData: string[][] = <any>await this.godleyIcon.table.getData();

    const flows = allData.slice(2).map(dataRow => ({
      name: dataRow[0]
    }));

    const rowSums = [await this.godleyIcon.rowSum(1)];
    for(let i = 0; i < flows.length; i++) rowSums.push(await this.godleyIcon.rowSum(i + 2));

    var columnVariableNames = (<string[]>allData[0]).slice(1);
    const columnVariables = [];
    let lastClass;
    for(let i = 0; i < columnVariableNames.length; i++) {
      const assetClass = <any>await this.godleyIcon.table._assetClass(i + 1);
      const newVar = {
        assetClass: assetClass,
        name: columnVariableNames[i]
      };
      columnVariables.push(newVar);

      if(assetClass !== lastClass) {
        columnVariables[columnVariables.length - 1].firstOfClass = true;
        if(i !== 0) {
          columnVariables[columnVariables.length - 2].lastOfClass = true;
        }
      }

      lastClass = assetClass;
    }

    const initialValues = [];
    for(let i = 0; i < columnVariables.length; i++) {
      initialValues.push(allData[1][i+1]);
    }

    this.flows = flows;
    this.columnVariables = columnVariables;
    this.cellValues = allData;
    this.initialValues = initialValues;
    this.rowSums = rowSums;

    this.htmlModeReady = true;

    this.cdRef.detectChanges();
  }

  async onCellFocus(i, j, event, handleFocus = true) {
    if(i !== this.cellEditing[0] || j !== this.cellEditing[1]) {
      await this.finishEditing();
    }

    this.cellEditing[0] = i;
    this.cellEditing[1] = j;

    if(handleFocus) {
      this.cdRef.detectChanges();
      
      this.focusInput(i,j);

      event.stopPropagation();
    }
  }

  focusInput(i, j) {
    const inputElement: HTMLInputElement = document.querySelector(`#dataCell${i}_${j} > input`);
      if(inputElement) inputElement.focus();
  }

  isCellEditing(i, j) {
    return this.cellEditing[0] === i && this.cellEditing[1] === j;
  }

  async finishEditing(clearEditing = true) {
    if(this.editingAnything()) {
      const editedValue = this.cellValues[this.cellEditing[0]][this.cellEditing[1]];
      
      this.godleyIcon.setCell(this.cellEditing[0], this.cellEditing[1], editedValue);

      if(clearEditing) {
        this.cellEditing[0] = undefined;
        this.cellEditing[1] = undefined;
      }

      await this.hardRefresh();
    }
  }

  delayedFinishEditing() {
    setTimeout(() => this.finishEditing(), 200);
  }

  editingAnything() {
    return this.cellEditing[0] !== undefined && this.cellEditing[1] !== undefined;
  }

  changeScale(e) {
    if(e.ctrlKey) {
      this.scale.changeScale(e.deltaY);
    }
  }

  onWedgeClick(columnIndex, event) {
    this.electronService.invoke(events.GODLEY_VIEW_IMPORT_STOCK, {
      command: this.itemId,
      columnIndex: columnIndex
    });
  }

  async inputKeyup(e: KeyboardEvent) {
    if(['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key) && this.editingAnything()) {
      switch(e.key) {
        case 'ArrowUp':
          e.stopPropagation();
          e.preventDefault();
          await this.finishEditing(false);
          this.cellEditing[0]--;
          this.cdRef.detectChanges();
          this.focusInput(this.cellEditing[0], this.cellEditing[1]);
        break;
        case 'ArrowDown':
          e.stopPropagation();
          e.preventDefault();
          await this.finishEditing(false);
          this.cellEditing[0]++;
          this.cdRef.detectChanges();
          this.focusInput(this.cellEditing[0], this.cellEditing[1]);
        break;
        case 'ArrowLeft':
          var element = e.target as HTMLInputElement;
          if(element.selectionStart == 0) {
            e.stopPropagation();
            e.preventDefault();
            await this.finishEditing(false);
            this.cellEditing[1]--;
            this.cdRef.detectChanges();
            this.focusInput(this.cellEditing[0], this.cellEditing[1]);
          }
        break;
        case 'ArrowRight':
          var element = e.target as HTMLInputElement;
          if(element.selectionStart == element.value.length) {
            e.stopPropagation();
            e.preventDefault();
            await this.finishEditing(false);
            this.cellEditing[1]++;
            this.cdRef.detectChanges();
            this.focusInput(this.cellEditing[0], this.cellEditing[1]);
          }
        break;
      }
    }
  }

  zoom(ratio: number) {
    this.zoomFactor*=ratio;
    document.body.style.setProperty('zoom', `${Math.round(this.zoomFactor*100)}%`);
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
