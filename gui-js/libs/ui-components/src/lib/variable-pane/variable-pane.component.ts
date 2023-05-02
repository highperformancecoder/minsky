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
  VariablePane,
} from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { fromEvent, Observable } from 'rxjs';
import { sampleTime } from 'rxjs/operators';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-variable-pane',
  templateUrl: './variable-pane.component.html',
  styleUrls: ['./variable-pane.component.scss'],
})
export class VariablePaneComponent implements OnDestroy, AfterViewInit {
  @ViewChild('variablePaneWrapper') variablePaneWrapper: ElementRef;

  itemId: number;
  systemWindowId: BigInt;

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  variablePaneContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;
  variablePane: VariablePane;
  
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
    this.topOffset = 20;
    
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

  initEvents() {

    this.mouseMove$ = fromEvent<MouseEvent>(
      this.variablePaneContainer,
      'mousemove'
    ).pipe(sampleTime(1)); /// FPS=1000/sampleTime

    // vertical offset to allow room for the filter buttons
    const offset=this.electronService.isMacOS()? 20: -15; // why, o why, Mac?
    this.mouseMove$.subscribe((event: MouseEvent) => {
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
  }

  onKeyDown =  (event: KeyboardEvent) => {
    if (event.shiftKey) {
      this.variablePane.shift(true);
      this.variablePane.requestRedraw();
    }
  };
  onKeyUp = (event: KeyboardEvent) => {
      if (event.shiftKey) {
        this.variablePane.shift(false);
        this.variablePane.requestRedraw();
      }
  };

  select(id) {
    if (document.forms["variablePane"]["variablePane::"+id].checked)
      this.variablePane.select(id);
    else
      this.variablePane.deselect(id);
    this.variablePane.updateWithHeight(this.height);
  }
    
  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
