import { Component, Input } from '@angular/core';
import { LatexDirective } from '../../directives/latex.directive';
import { CommonModule } from '@angular/common';

@Component({
    selector: 'elementlabel',
    templateUrl: './elementlabel.component.html',
    styleUrls: ['./elementlabel.component.css'],
    standalone: true,
    imports: [LatexDirective, CommonModule]
  })
  export class ElementLabelComponent {
    @Input()
    data;

    @Input()
    lines: number;

    get fontSize() {
        return `${20 / this.lines}px`;
    }
  }