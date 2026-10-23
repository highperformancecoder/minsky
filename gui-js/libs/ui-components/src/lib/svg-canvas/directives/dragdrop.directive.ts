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
            .on('start', (event) => df.dragstarted(event))
            .on('drag', (event) => df.dragged(event))
            .on('end', (event) => df.dragended(event)));
    }

    constructor(private el: ElementRef<HTMLElement>) { 
    
    }
}