import {
  AfterViewInit,
  Component,
  ElementRef,
  OnDestroy,
  ViewChild,
} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {
  ElectronService,
} from '@minsky/core';
import {
  events,
  isWindows,
  VariablePane,
} from '@minsky/shared';
import { fromEvent, Observable, Subject, takeUntil } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@Component({
  selector: 'minsky-variable-pane',
  templateUrl: './variable-pane.component.html',
  styleUrls: ['./variable-pane.component.scss'],
  standalone: true
})
export class VariablePaneComponent implements OnDestroy, AfterViewInit {
  @ViewChild('variablePane') variablePaneWrapper: ElementRef;

  itemId: number;
  systemWindowId: BigInt;

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  variablePaneContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;
  variablePane: VariablePane;
  
  destroy$ = new Subject<{}>();

  mouseX = 0;
  mouseY = 0;

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
    this.variablePane=electronService.minsky.variablePane;
  }

  ngAfterViewInit() {
    this.getWindowRectInfo();
    this.renderFrame();
    this.initEvents();
  }

  windowResize() {
    this.getWindowRectInfo();
    this.renderFrame();
  }
  private getWindowRectInfo() {
    this.variablePaneContainer = this.variablePaneWrapper
      .nativeElement as HTMLElement;

    const clientRect = this.variablePaneContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);
    // 20 pixel offset to allow for selector buttons. Not needed on Windows for some reason.
    this.topOffset = isWindows()? 0: 20; 
    
    this.height = Math.round(this.variablePaneContainer.clientHeight);
    this.width = Math.round(this.variablePaneContainer.clientWidth-this.topOffset);
  }
  
  renderFrame() {
    if (
      this.electronService.isElectron &&
      this.systemWindowId &&
      this.height &&
      this.width
    ) {
      this.variablePane.updateWithHeight(this.height);
      this.variablePane.renderFrame({
        parentWindowId: this.systemWindowId.toString(),
        offsetLeft: this.leftOffset,
        offsetTop: this.topOffset,
        childWidth: this.width,
        childHeight: this.height,
        scalingFactor: -1
      });
    }
  }

  async initEvents() {

    this.mouseMove$ = fromEvent<MouseEvent>(
      this.variablePaneContainer,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    // vertical offset to allow room for the filter buttons
    const offset=-15;
    this.mouseMove$.pipe(takeUntil(this.destroy$)).subscribe((event: MouseEvent) => {
      const { clientX, clientY } = event;
      this.mouseX = clientX;
      this.mouseY = clientY;
      this.variablePane.mouseMove(clientX,clientY+offset);
      this.variablePane.requestRedraw();
    });

    this.variablePaneContainer.addEventListener('mousedown', (event) => {
      const { clientX, clientY } = event;
      this.variablePane.mouseDown(clientX,clientY+offset);
      this.variablePane.requestRedraw();
    });

    this.variablePaneContainer.addEventListener('mouseup', (event) => {
      const { clientX, clientY } = event;
      this.variablePane.mouseUp(clientX,clientY+offset);
      this.variablePane.requestRedraw();
    });

//    this.variablePaneContainer.onwheel = this.onMouseWheelZoom;
    document.onkeydown = this.onKeyDown;
    document.onkeyup = this.onKeyUp;

    // set initial value of form elements from C++
    let selected=await this.variablePane.selection.properties();
    for (let i of ['flow','parameter','stock','integral'])
      document.forms["variablePane"]["variablePane::"+i].checked=selected.includes(i);
  }

  onKeyDown =  (event: KeyboardEvent) => {
    if (event.shiftKey) {
      document.body.style.cursor='grab';
      this.variablePane.shift(true);
      this.variablePane.requestRedraw();
    }
    if (event.code==='F1')
      this.electronService.send(events.HELP_FOR, {classType:'VariableBrowser'});
  };
  onKeyUp = (event: KeyboardEvent) => {
    document.body.style.cursor='default';
    this.variablePane.shift(false);
    this.variablePane.requestRedraw();
  };

  select(id) {
    if (document.forms["variablePane"]["variablePane::"+id].checked)
      this.variablePane.select(id);
    else
      this.variablePane.deselect(id);
    this.variablePane.updateWithHeight(this.height);
  }
    
  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
