import { Directive, ElementRef, Input, OnChanges } from '@angular/core';

declare var MathJax;

@Directive({
  selector: '[latex]'
})
export class LatexDirective implements OnChanges {
    @Input()
    equation: string;

    @Input()
    latexScale: number = 1;

    constructor(private el: ElementRef<HTMLElement>) { 
    
    }

    ngOnChanges(changes) {
      const html: HTMLElement = MathJax.tex2svg(this.equation, {em: 12, ex: 6, display: false}).firstChild;
      (<any>html).style.transform = `scale(${this.latexScale}, ${this.latexScale})`;
      this.el.nativeElement.innerHTML = '';
      this.el.nativeElement.append(html);
    }
}