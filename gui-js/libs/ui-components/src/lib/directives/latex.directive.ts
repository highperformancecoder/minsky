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
 
    @Input() 
    replaceSpaces = false; 
 
    constructor(private el: ElementRef<HTMLElement>) {  
     
    } 
 
    ngOnChanges(changes) { 
      let equationValue = this.equation; 
      if(this.replaceSpaces) equationValue = equationValue.replace(/ /g, '\\,'); 
      const html: HTMLElement = MathJax.tex2svg(equationValue, {em: 12, ex: 6, display: false}).firstChild; 
      (<any>html).style.transform = `scale(${this.latexScale}, ${this.latexScale})`; 
      this.el.nativeElement.innerHTML = ''; 
      this.el.nativeElement.append(html); 
    } 
}