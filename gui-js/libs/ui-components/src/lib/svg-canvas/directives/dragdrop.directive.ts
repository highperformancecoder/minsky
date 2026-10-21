import { Directive, ElementRef, Input } from '@angular/core';
import * as d3 from 'd3';

@Directive({
  selector: '[dragdrop]'
})
export class DragdropDirective {
    @Input()
    set dragfunctions(df) {
      let obj = d3.select(this.el.nativeElement);

        obj.call(<any>d3.drag()
            .on('start', (d, e) => df.dragstarted(d))
            .on('drag', (d, e) => df.dragged(d))
            .on('end', (d, e) => df.dragended(d)));
    }

    constructor(private el: ElementRef<HTMLElement>) { 
    
    }
}