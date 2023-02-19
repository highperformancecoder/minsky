import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  ElementRef,
  OnDestroy,
  OnInit,
  ViewChild,
  HostListener
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
import { fromEvent, Observable, of } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

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

  // in canvas mode, the C++ layer writes to the godley-cairo-canvas element
  // this can be re-enabled to compare the visual output and interaction between the canvas based renderer and the HTML based one
  // switching this interactively without reinitialzing the window doesn't work, because the C++ output is very persistent. may need to clear it somehow.
  canvasMode = false;

  htmlModeReady = false;

  assetVariables = [];
  liabilityVariables = [];
  equityVariables = [];
  columnVariables = [];
  flows = [];

  initialValues = [];
  rowSums = [];
  cellValues = [];

  cellEditing = [undefined, undefined];

  // keep track of scale and zoom factors separately, because using zoom exclusively creates rounding errors
  scaleBase = 1.1;
  scalePower = 1;
  scaleStep = 1 / 53;
  zoomFactor = 1;

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

    await this.hardRefresh();
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
        type: "godley",
        command: this.godleyIcon.prefix(),
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

  async onRowAdd(i) {
    if(i >= 0 && i < this.flows.length) {
      this.godleyIcon.table.insertRow(i+3);
      //this.variables.splice(i + 1, 0, "new");
      await this.hardRefresh();
    }
  }

  async onRowDelete(i) {
    if(i >= 0 && i < this.flows.length) {
      this.godleyIcon.deleteRow(i+3);
      //this.variables.splice(i, 1);
      await this.hardRefresh();
    }
  }

  async onRowMove(i, n) {
    const resultIndex = i + n;
    if(resultIndex >= 0 && resultIndex < this.flows.length) {
      this.godleyIcon.table.moveRow(i+2,n);
      //const deleted = this.variables.splice(i, 1);
      //this.variables.splice(i+n, 0, deleted[0]);
      await this.hardRefresh();
    }
  }

  async onColumnAdd(i) {
    if(i >= 0 && i < this.columnVariables.length) {
      this.godleyIcon.table.insertCol(i+1);
      //this.columnVariables.splice(i + 1, 0, "new");
      await this.hardRefresh();
    }
  }

  async onColumnDelete(i) {
    if(i >= 0 && i < this.columnVariables.length) {
      this.godleyIcon.table.deleteCol(i+1);
      //this.columnVariables.splice(i, 1);
      await this.hardRefresh();
    }
  }

  async onColumnMove(i, n) {
    const resultIndex = i + n;
    if(resultIndex >= 0 && resultIndex < this.flows.length) {
      this.godleyIcon.table.moveCol(i+1,n);
      //const deleted = this.columnVariables.splice(i, 1);
      //this.columnVariables.splice(i+n, 0, deleted[0]);
      await this.hardRefresh();
    }
  }

  async hardRefresh(update = true) {
    if(update) this.godleyIcon.update();

    const allData: string[][] = <any>await this.godleyIcon.table.getData();

    const flows = allData.slice(2).map(dataRow => ({
      name: dataRow[0]
    }));

    const rowSums = [await this.godleyIcon.rowSum(1)];
    for(let i = 0; i < flows.length; i++) rowSums.push(await this.godleyIcon.rowSum(i + 2));

    var columnVariableNames = (<string[]>allData[0]).slice(1);
    const columnVariables = [];
    const assetVariables = [];
    const liabilityVariables = [];
    const equityVariables = [];
    let lastClass;
    for(let i = 0; i < columnVariableNames.length; i++) {
      const assetClass = <any>await this.godleyIcon.table._assetClass(i + 1);
      const newVar = {
        assetClass: assetClass,
        name: columnVariableNames[i]
      };
      columnVariables.push(newVar);

      if(`${assetClass}` === 'asset') assetVariables.push(newVar);
      if(`${assetClass}` === 'liability') liabilityVariables.push(newVar);
      if(`${assetClass}` === 'equity') equityVariables.push(newVar);

      if(assetClass !== lastClass) {
        columnVariables[columnVariables.length - 1].firstOfClass = true;
        if(i !== 0) {
          columnVariables[columnVariables.length - 2].lastOfClass = true;
        }
      }

      lastClass = assetClass;
    }

    const cellValues = [];
    for(let i = 0; i < flows.length; i++) {
      const cellRow = [];
      for(let j = 0; j < columnVariables.length; j++) {
        cellRow.push(allData[i + 2][j + 1]);
      }
      cellValues.push(cellRow);
    }

    const initialValues = [];
    for(let i = 0; i < columnVariables.length; i++) {
      initialValues.push(allData[1][i+1]);
    }

    this.flows = flows;
    this.columnVariables = columnVariables;
    this.assetVariables = assetVariables;
    this.liabilityVariables = liabilityVariables;
    this.equityVariables = equityVariables;
    this.cellValues = cellValues;
    this.initialValues = initialValues;
    this.rowSums = rowSums;

    this.htmlModeReady = true;
  }

  async onCellFocus(i, j, event, handleFocus = true) {
    if(i !== this.cellEditing[0] || j !== this.cellEditing[1]) {
      await this.finishEditing();
    }

    this.cellEditing[0] = i;
    this.cellEditing[1] = j;

    if(handleFocus) {
      this.cdRef.detectChanges();
      
      const inputElement: HTMLInputElement = document.querySelector(`#dataCell${i}_${j} > input`);
      if(inputElement) inputElement.focus();

      event.stopPropagation();
    }
  }

  isCellEditing(i, j) {
    return this.cellEditing[0] === i && this.cellEditing[1] === j;
  }

  async finishEditing() {
    if(this.editingAnything()) {
      let editedValue;
      if(this.cellEditing[0] === -1) { // initial conditions row
        editedValue = this.initialValues[this.cellEditing[1]];
      } else if(this.cellEditing[0] === -2) { // column variable names
        editedValue = this.columnVariables[this.cellEditing[1]].name;
      } else if(this.cellEditing[1] === -1) { // row variable names
        editedValue = this.flows[this.cellEditing[0]].description;
      } else {
        editedValue = this.cellValues[this.cellEditing[0]][this.cellEditing[1]];
      }
      this.godleyIcon.setCell(this.cellEditing[0] + 2, this.cellEditing[1] + 1, editedValue);

      this.cellEditing[0] = undefined;
      this.cellEditing[1] = undefined;
      await this.hardRefresh();
    }
  }

  editingAnything() {
    return this.cellEditing[0] !== undefined && this.cellEditing[1] !== undefined;
  }

  changeScale(e) {
    if(e.ctrlKey) {
      this.scalePower -= e.deltaY * this.scaleStep;

      this.zoomFactor = Math.pow(this.scaleBase, this.scalePower);
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
