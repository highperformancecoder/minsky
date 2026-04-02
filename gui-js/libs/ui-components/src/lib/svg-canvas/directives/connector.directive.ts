import { Directive, ElementRef, HostListener } from '@angular/core';

@Directive({
  selector: '[connector]'
})
export class ConnectorDirective {
  @HostListener('mouseenter', ['$event'])
  mouseenter(event: any) {
    this.el.nativeElement.style.fillOpacity = '0.5';
  }

  @HostListener('mouseout', ['$event'])
  mouseout(event: any) {
    this.el.nativeElement.style.fillOpacity = '0';
  }

  constructor(private el: ElementRef<HTMLElement>) { 
    el.nativeElement.style.fillOpacity = '0';
  }
}