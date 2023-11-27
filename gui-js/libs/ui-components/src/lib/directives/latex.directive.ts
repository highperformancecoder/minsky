import { Directive, ElementRef, Input, OnChanges } from '@angular/core';
import { get_mathjax_svg } from '@minsky/ui-components';

@Directive({
  selector: '[latex]'
})
export class LatexDirective implements OnChanges {
    @Input()
    equation: string;

    @Input()
    latexScale: number = 1;

    @Input()
    replaceSpaces = false;

    constructor(private el: ElementRef<HTMLElement>) { 
    
    }

    ngOnChanges(changes) {
      let equationValue = this.equation;
      if(this.replaceSpaces) equationValue = equationValue.replace(/ /g, '\\,');
      const svgstring = get_mathjax_svg(equationValue, {em: 12, ex: 6, display: false});
      this.el.nativeElement.innerHTML = svgstring;
      (<HTMLElement>this.el.nativeElement.firstChild).style.transform = `scale(${this.latexScale}, ${this.latexScale})`;
    }
}