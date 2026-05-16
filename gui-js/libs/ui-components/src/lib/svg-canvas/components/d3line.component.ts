import { CommonModule } from '@angular/common';
import { ElementRef, Input, OnChanges, ViewChild, Component, SimpleChanges, Output, EventEmitter, ViewChildren } from '@angular/core';
import * as d3 from 'd3';

@Component({
  selector: '[d3line]',
  templateUrl: './d3line.component.html',
    styleUrls: ['./d3line.component.css'],
    imports: [CommonModule]
})
export class D3LineComponent implements OnChanges {
    @ViewChild('linepath', {static: true})
    linepath: ElementRef;

    @ViewChild('startpoint', {static: true})
    startpoint: ElementRef;

    @ViewChild('endpoint', {static: true})
    endpoint: ElementRef;

    @ViewChildren('midpoint')
    midpoints;

    @Input()
    start: [number,number,number];

    @Input()
    end: [number,number,number];

    @Input()
    dotsAt: number[];

    dotPositions: any[] = [];

    @Input()
    invisibleLine = false;

    @Input()
    draggable = true;

    @Output()
    pointMoved = new EventEmitter<any>();

    constructor() {
    
    }

    ngOnChanges(changes: SimpleChanges) {
        if(this.start && this.end && (changes.start || changes.end)) {
            const points: any = [this.start];
            
            const pointDistance = Math.sqrt(Math.pow(this.end[0] - this.start[0], 2) + Math.pow(this.end[1] - this.start[1], 2)) / 4;
            if(this.start[2] !== undefined) {
                points.push([this.start[0] + Math.cos(this.start[2]) * pointDistance, this.start[1] + Math.sin(this.start[2]) * pointDistance]);
            }
            if(this.end[2] !== undefined) {
                points.push([this.end[0] + Math.cos(this.end[2]) * pointDistance, this.end[1] + Math.sin(this.end[2]) * pointDistance]);
            }
            points.push(this.end);
            
            this.linepath.nativeElement.setAttribute('d', d3.line().curve(d3.curveBasis)(points));
        }

        if(changes.dotsAt) {
            this.handleDotsChange();
        }
    }

    handleDotsChange() {
        const path = this.linepath.nativeElement;
        const length = path.getTotalLength();
        this.dotPositions = this.dotsAt.map(percentage => {
            return path.getPointAtLength(percentage * length);
        });
    }

    dragfunctions(p, i) {
        const startBox = this.startpoint.nativeElement.getBBox();
        const endBox = this.endpoint.nativeElement.getBBox();
        const start = [startBox.x + startBox.width / 2, startBox.y + startBox.height / 2];
        const end = [endBox.x + endBox.width / 2, endBox.y + endBox.height / 2];

        return {
          dragstarted: () => {
          },
          dragged: () => {
            if(this.draggable) {
                const x = d3.event.x;
                const y = d3.event.y;
                const point = [x,1000];

                const a1 = Math.max(0, Math.PI / 2 - this.getAngle(end, start, point));
                const a2 = Math.max(0, Math.PI / 2 - this.getAngle(start, end, point));

                this.dotsAt[i] = a1 / (a1 + a2);
                this.pointMoved.emit(this.dotsAt[i]);
            }
          },
          dragended: () => {
            
          }
        };
    }

    getAngle(x, y, z) {
        const a = [z[0] - y[0], z[1] - y[1]];
        const b = [x[0] - y[0], x[1] - y[1]];
        const al = Math.sqrt(Math.pow(a[0],2)+Math.pow(a[1],2));
        const bl = Math.sqrt(Math.pow(b[0],2)+Math.pow(b[1],2));
        const dotproduct = a[0] * b[0] + a[1] * b[1];
        return Math.acos(dotproduct / (al * bl));
    }
}