import { Component } from '@angular/core';
import { SvgCanvasComponent } from './svg-canvas.component';
import { SvgCanvasHelper, SvgVariableTypes } from './constants/svg-constants';

@Component({
  selector: 'svg-canvas-demo',
  templateUrl: './svg-canvas-demo.component.html',
  styleUrls: ['./svg-canvas-demo.component.css'],
  standalone: true,
  imports: [SvgCanvasComponent]
})
export class SvgCanvasDemoComponent {
    parkedData = [
      { symbol: 'C_{const}', name: 'Constant', description: 'A constant has a fixed value that does not change over time, unless manipulated by the user.', type: SvgVariableTypes.constant },
      { symbol: 'V_{stock}', name: 'Stock Variable', description: 'A stock variable has a value that can be approximated based on the slope defined by its input.', type: SvgVariableTypes.stock }, 
      { symbol: 'V_{flow}', name: 'Flow Variable', description: 'A flow variable has a value that is derived directly from its input.', type: SvgVariableTypes.flow }, 
      { symbol: '\\times', name: 'Times', type: SvgVariableTypes.operator }, 
      { symbol: '+', name: 'Plus', type: SvgVariableTypes.operator },
      { symbol: '-', name: 'Minus', type: SvgVariableTypes.operator },
      { symbol: '\\:/', name: 'Divide', type: SvgVariableTypes.operator }
    ].map((template: any, i: number) => {
      const d = SvgCanvasHelper.createData(template);
      d.x = 60 + 120 * i;
      d.y = 40;
      d.rotation = 0;
      d.value = 1;
      d.minimum = 0.5;
      d.maximum = 2;
      return d;
    });

    unparkedData: any[] = [];
}
