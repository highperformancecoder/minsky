import { CommonModule } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectionStrategy,
  Component,
  ElementRef,
  HostListener,
  OnDestroy,
  ViewChild,
} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import {
  ElectronService,
} from '@minsky/core';
import {
  VariablePane,
  VariableValue,
  VariableValues,
  minsky
} from '@minsky/shared';
import { BehaviorSubject, Observable, Subject, takeUntil } from 'rxjs';
import { SvgCanvasComponent } from '../svg-canvas/svg-canvas.component';
import { SvgCanvasHelper, SvgVariableTypes } from '../svg-canvas/constants/svg-constants';
import { DataPoint } from '../svg-canvas/classes/datapoint';

@Component({
  selector: 'minsky-variable-pane',
  templateUrl: './variable-pane.component.html',
  styleUrls: ['./variable-pane.component.scss'],
  standalone: true,
  imports: [CommonModule, SvgCanvasComponent],
  changeDetection: ChangeDetectionStrategy.OnPush
  
})
export class VariablePaneComponent implements OnDestroy, AfterViewInit {
  @ViewChild('variablePane') variablePaneWrapper: ElementRef;

  @ViewChild(SvgCanvasComponent)
  svgCanvas: SvgCanvasComponent;

  @HostListener('window:resize', ['$event'])
	onResize(event) {
		this.setPositions();
	}

  itemId: number;
  systemWindowId: BigInt;

  allSvgData: DataPoint[];

  svgData$ = new BehaviorSubject<DataPoint[]>(null);

  leftOffset = 0;
  topOffset: number;
  height: number;
  width: number;
  variablePaneContainer: HTMLElement;
  mouseMove$: Observable<MouseEvent>;
  variablePane: VariablePane;

  containerWidth$ = new BehaviorSubject<string>(null);
  
  destroy$ = new Subject<{}>();

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
    this.initEvents();
  }

  windowResize() {
    this.variablePaneContainer = this.variablePaneWrapper
      .nativeElement as HTMLElement;
  }

  async initEvents() {
    this.variablePaneContainer = this.variablePaneWrapper
      .nativeElement as HTMLElement;

    // Translate vertical wheel movement (deltaY) to horizontal scrolling
    this.variablePaneContainer.addEventListener('wheel', (evt) => {
      evt.preventDefault();
      this.variablePaneContainer.scrollLeft += evt.deltaY;
    }, { passive: false });

    this.height = Math.round(this.variablePaneContainer.clientHeight);
    this.variablePane.updateWithHeight(this.height);
    // set initial value of form elements from C++
    let selected=await this.variablePane.selection.properties();
    const supportedTypes = ['flow','parameter','stock','integral'];
    for (let i of supportedTypes)
      document.forms['variablePane']['variablePane::'+i].checked=selected.includes(i);

    this.allSvgData = [];
    const varValues: VariableValues = await minsky.variableValues;
    const varKeys = await varValues.keys();
    for(let i = 0; i < varKeys.length; i++) {
      const varValue: VariableValue = varValues.elem(varKeys[i]);
      const typeName = await varValue.type();
      if(!supportedTypes.some(t => t === typeName)) continue;
      let trimmedName;
      if(typeName === 'constant') {
        continue;
      } else {
        trimmedName = (await varValue.name());
        const splitName = trimmedName.split(':');
        if(splitName.length > 1) {
          trimmedName = splitName[1];
        }
      }
      const d = SvgCanvasHelper.createData({ symbol: trimmedName, name: trimmedName, description: await varValue.detailedText(), type: SvgVariableTypes[typeName] });
      d.rotation = 0;
      d.value = await varValue.value();
      d.exponent = this.findExponent(d.value);
      d.value = d.value / Math.pow(10, d.exponent);
      d.minimum = await varValue.sliderMin();
      d.maximum = await varValue.sliderMax();
      this.allSvgData.push(d);
    }
    this.setPositions();
  }

  setPositions() {
    const svgData = this.allSvgData.filter(d => {
      return document.forms['variablePane']['variablePane::'+d.type.name].checked;
    });
    svgData.sort((a,b) => {
      if(a.type.name > b.type.name) return 1;
      if(a.type.name < b.type.name) return -1;
      if(a.symbol > b.symbol) return 1;
      if(a.symbol < b.symbol) return -1;
      return 0;
    });

    const columnLength = Math.round(this.variablePaneContainer.clientHeight / 50);
    const numberOfColumns = svgData.length / columnLength;
    let currentXOffset = 10;
    for(let i = 0; i < numberOfColumns; i++) {
      let maxX = 0;
      for(let j = 0; j < columnLength; j++) {
        const d = svgData[i * columnLength + j];
        if(!d) break;
        maxX = Math.max(maxX, d.dimensions.boundingbox[2]);
      }
      for(let j = 0; j < columnLength; j++) {
        const d = svgData[i * columnLength + j];
        if(!d) break;
        d.x = currentXOffset - d.dimensions.boundingbox[0];
        d.y = 40 + 60 * j;
      }
      currentXOffset += maxX + 20;
    }
    this.containerWidth$.next(`${currentXOffset}px`);
    this.svgData$.next(svgData);
    this.svgCanvas.setDimensions(currentXOffset);
  }

  select(id) {
    if (document.forms["variablePane"]["variablePane::"+id].checked) this.variablePane.select(id);
    else this.variablePane.deselect(id);
    this.setPositions();
  }

  findExponent(num: number) {
    if (num === 0) return 0;
    const absNum = Math.abs(num);
    const rawExponent = Math.floor(Math.log10(absNum));
    return Math.floor(rawExponent / 3) * 3;
  }
    
  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
