import { Component, AfterViewInit, ElementRef, HostListener, Input } from '@angular/core';
import * as d3 from 'd3';
import { DataPoint } from './classes/datapoint';
import { EditVariableDialog, EditVariableDialogData } from './dialogs/editvariabledialog.component';
import { MatDialog } from '@angular/material/dialog';
import flatMap from 'lodash/flatMap';
import { AsyncPipe, CommonModule, formatNumber } from '@angular/common';
import { MatFormField } from '@angular/material/input';
import { LatexDirective } from '../directives/latex.directive';
import { DragdropDirective } from './directives/dragdrop.directive';
import { D3LineComponent } from './components/d3line.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ElementLabelComponent } from './components/elementlabel.component';
import { NgxGraphModule } from '@swimlane/ngx-graph';
import { SvgCanvasHelper } from './constants/svg-constants';

@Component({
  selector: 'svg-canvas',
  templateUrl: './svg-canvas.component.html',
  styleUrls: ['./svg-canvas.component.css'],
  standalone: true,
  imports: [AsyncPipe, CommonModule, MatFormField, LatexDirective, DragdropDirective, D3LineComponent, FormsModule, ReactiveFormsModule, ElementLabelComponent, NgxGraphModule]
})
export class SvgCanvasComponent implements AfterViewInit {
  @HostListener('window:resize', ['$event'])
	onResize(event) {
    this.setDimensions();
	}

  nodes;

  width: number;
  height: number;
  simulation;

  @Input()
  parkedData: any[] = [];

  @Input()
  unparkedData: any[] = [];

  @Input()
  enableScroll = true;

  @Input()
  showConnectors = true;

  @Input()
  showValuelines = true;

  @Input()
  zoom = 1;

  rectCollideForce;

  xScale;
  yScale;
  zScale;
  storedXScale;
  storedYScale;

  draggedElement;

  rotationMouseX;

  screenmovestart: number[];

  clickedConnector;
  lineend: [number, number];

  links = [];

  mouseX: number;
  mouseY: number;

  controlDown = false;

  layoutType = 'spread';

  factorLevels = [1.5, 3];

  getLines(yFactor: number) {
    return Math.min(Math.ceil((yFactor + 0.75) / 1.5), 4);
  }

  roundValue(value: number) {
    return formatNumber(value, 'en-US', '1.0-2');
  }

  exponentEquation(exponent: number) {
    return `\\times10^{${exponent}}`;
  }

  isUpsideDown(d) {
    if(d.type.name === 'operator') return false;
    const quarterAdded = d.rotation + Math.PI * 0.5;
    return this.posMod(quarterAdded, Math.PI * 2) > Math.PI;
  }

  posMod(radians, div) {
    let mod = radians % div;
    while(mod < 0) mod += div;
    return mod;
  }

  get xFactor() {
    const range = this.xScale.range();
    return (range[1] - range[0]) / this.width;
  }

  get yFactor() {
    const range = this.yScale.range();
    return (range[1] - range[0]) / this.height;
  }

  getRelativeValue(d, bounded = true) {
    const relValue = (d.value - d.minimum) / (d.maximum - d.minimum);
    if(!bounded) return relValue;
    return Math.max(0, Math.min(1, relValue));
  }

  setRelativeValue(d, relativeValue: number) {
    d.value = d.minimum + (d.maximum - d.minimum) * relativeValue;
  }

  types: any[];

  constructor(private el: ElementRef<HTMLElement>, public dialog: MatDialog) {
    
  }

  setDimensions(width?: number) {
    this.width = width || this.el.nativeElement.clientWidth;
    this.height = this.el.nativeElement.clientHeight;

    this.xScale = d3.scaleLinear()
    .domain([0, this.width])
    .range([0, this.width * this.zoom]);

    this.yScale = d3.scaleLinear()
    .domain([0, this.height])
    .range([0, this.height * this.zoom]);

    this.zScale = d3.scaleLinear()
    .domain([0, 0])
    .range([0, 0]);
  }

  ngAfterViewInit() {
    this.setDimensions();

    /*this.rectCollideForce = this.rectCollide();
    this.rectCollideForce.strength(3);

    this.simulation = d3.forceSimulation()
    .force('rectcollide', this.rectCollideForce)
    .nodes(this.unparkedData);*/
  }

  setForces() {
    this.simulation
    ?.force('anchorx', d3.forceX().x((d: DataPoint) => d.anchorx || d.x).strength((d: DataPoint) => d.anchorx ? 15 : 0))
    .force('anchory', d3.forceY().y((d: DataPoint) => d.anchory || d.y).strength((d: DataPoint) => d.anchory ? 15 : 0))
  }

  dragfunctions(d) {
    return {
      dragstarted: () => {
        if(this.clickedConnector) return;

        if(this.layoutType === 'flat') {
          this.spreadLayout();
          this.flattenLayout(d, 2);
          return;
        }

        this.draggedElement = d;
        if(this.rotationMouseX) this.rotationMouseX = this.mouseX;

        if (!d3.event.active) this.simulation?.alphaTarget(.03).restart();
        if(this.parkedData.includes(d)) {
          const clone = SvgCanvasHelper.createData(d);
          this.parkedData.push(clone);
        }
        this.setAnchor(d);

        this.rectCollideForce.sizeModifier(1.1);
      },
      dragged: () => {
        if(this.clickedConnector || this.layoutType !== 'spread') return;

        const parked = this.parkedData.includes(d);
        
        if(parked) {
          d.fx = d3.event.x;
          d.fy = d3.event.y;

          d.x = d.fx;
          d.y = d.fy;
        } else {
          if(this.rotationMouseX) {
            if(d.startRotation === undefined) d.startRotation = d.rotation;
            d.rotation = this.posMod(d.startRotation + (d3.event.x - this.rotationMouseX) / 100, Math.PI * 2);
            const divider = Math.PI / 4;
            const treshold = Math.PI / 32;
            const diff = this.posMod(d.rotation, divider);
            if(diff < treshold) d.rotation -= diff;
            else if(diff > (divider - treshold)) d.rotation += divider - diff;
          } else {
            d.fx = this.xScale.invert(d3.event.x);
            d.fy = this.yScale.invert(d3.event.y);
          }
        }
        
        if(parked && d.y > 80 + d.dimensions.boundingbox[1] + d.dimensions.boundingbox[3]) {
          this.unparkedData.push(d);
          d.fx = this.xScale.invert(d3.event.x);
          d.fy = this.yScale.invert(d3.event.y);
          this.parkedData = this.parkedData.filter(pd => pd !== d);
          this.simulation?.nodes(this.unparkedData);
        }
      },
      dragended: () => {
        if(this.clickedConnector || this.layoutType !== 'spread') return;

        d.startRotation = undefined;

        this.draggedElement = null;

        if (!d3.event.active) this.simulation?.alphaTarget(.03);

        // remove if dragged onto the toolbar
        this.parkedData = this.parkedData.filter(pd => pd !== d);

        this.setAnchor(d);

        setTimeout(() => {
          this.setAnchor(d);

          this.rectCollideForce.sizeModifier(1);
        }, 100);
      }
    };
  }

  showParked(d){
    console.info(d);
    return '';
  }

  setAnchor(d) {
    this.unparkedData.forEach(n => {
      n.anchorx = n.x;
      n.anchory = n.y;
    });
    d.fx = null;
    d.fy = null;
    this.setForces();
  }

  rectCollide() {
    var nodes;
    var strength = 1
    var iterations = 1
    var sizeModifier = 1;

    const adjustSize = (boundingbox, rotation) => {
      const hp = [boundingbox[2] / 2, boundingbox[3] / 2];
      const rotated1 = this.rotatePoint(hp, rotation);
      const rotated2 = this.rotatePoint([hp[0],-hp[1]], rotation);
      const output = [Math.max(Math.abs(rotated1[0]) + Math.abs(rotated2[0])) * sizeModifier, Math.max(Math.abs(rotated1[1]) + Math.abs(rotated2[1])) * sizeModifier];
      return output;
    };

    function force() {
        var node, size, xi, yi
        var i = -1
        while (++i < iterations) { iterate() }

        

        function iterate() {
            var j = -1
            var tree = d3.quadtree(nodes, xCenter, yCenter).visitAfter(prepare)

            while (++j < nodes.length) {
                node = nodes[j]
                size = adjustSize(node.type.boundingbox, node.rotation)
                xi = xCenter(node)
                yi = yCenter(node)

                tree.visit(apply)
            }
        }

        function apply(quad, x0, y0, x1, y1) {
            var data = quad.data
            var xSize = (size[0] + quad.size[0]) / 2
            var ySize = (size[1] + quad.size[1]) / 2
            if (data) {
                if (data.index <= node.index) { return }

                var x = xi - xCenter(data)
                var y = yi - yCenter(data)
                var xd = Math.abs(x) - xSize
                var yd = Math.abs(y) - ySize

                if (xd < 0 && yd < 0) {
                    var l = Math.sqrt(x * x + y * y)
                    var mass = data.type.boundingbox[2] * data.type.boundingbox[3]
                    var m = mass / (mass + mass)

                    if (Math.abs(xd) < Math.abs(yd)) {
                        node.vx -= (x *= xd / l * strength) * m
                        data.vx += x * (1 - m)
                    } else {
                        node.vy -= (y *= yd / l * strength) * m
                        data.vy += y * (1 - m)
                    }
                }
            }

            return x0 > xi + xSize || y0 > yi + ySize ||
                   x1 < xi - xSize || y1 < yi - ySize
        }

        function prepare(quad) {
            if (quad.data) {
                quad.size = adjustSize(quad.data.type.boundingbox, quad.data.rotation)
            } else {
                quad.size = [0, 0]
                var i = -1
                while (++i < 4) {
                    if (quad[i] && quad[i].size) {
                        quad.size[0] = Math.max(quad.size[0], quad[i].size[0])
                        quad.size[1] = Math.max(quad.size[1], quad[i].size[1])
                    }
                }
            }
        }
    }

    function xCenter(d) { return d.x + d.vx }
    function yCenter(d) { return d.y + d.vy }

    force.initialize = function (_) {
        nodes = _
    }

    force.sizeModifier = function (_) {
        sizeModifier = _;
    }

    force.strength = function (_) {
        return (arguments.length ? (strength = +_, force) : strength)
    }

    force.iterations = function (_) {
        return (arguments.length ? (iterations = +_, force) : iterations)
    }

    return force
  }

  constant(_) {
    return () => _;
  }

  @HostListener('mousewheel', ['$event'])
  scroll(event: any) {
    if(!this.enableScroll || this.layoutType === 'flat') return;

    const wheelDelta = Math.max(-1, Math.min(1, (event.wheelDelta || -event.detail)));
    const change = wheelDelta > 0 ? 1.1 : (1/1.1);
    const xRange = this.xScale.range();
    const oldXSize = xRange[1] - xRange[0];
    const xSize = oldXSize * change;
    const yRange = this.yScale.range();
    const oldYSize = yRange[1] - yRange[0];
    const ySize = oldYSize * change;
    const newCenterX = oldXSize / 2 + xRange[0];
    const newCenterY = oldYSize / 2 + yRange[0];

    this.xScale.range([newCenterX - xSize / 2, newCenterX + xSize / 2]);
    this.yScale.range([newCenterY - ySize / 2, newCenterY + ySize / 2]);
  }

  @HostListener('mousedown', ['$event'])
  mousedown(event: any) {
    this.mouseX = this.getMouseX(event);
    this.mouseY = this.getMouseY(event);
    if(event.button === 1) {
      this.screenmovestart = [this.xScale(this.mouseX), this.yScale(this.mouseY)];
    }
    if(event.button === 2) {
      if(this.layoutType === 'flat') {
        this.spreadLayout();
        return;
      }
      this.lineend = null;
      this.clickedConnector = null;
      event.preventDefault();
      event.stopPropagation();
    }
  }
  @HostListener('mousemove', ['$event'])
  mousemove(event: any) {
    this.mouseX = this.getMouseX(event);
    this.mouseY = this.getMouseY(event);
    if(this.screenmovestart) {
      const newPos = [this.xScale(this.mouseX), this.yScale(this.mouseY)];
      const xDiff = newPos[0] - this.screenmovestart[0];
      const yDiff = newPos[1] - this.screenmovestart[1];
      const xRange = this.xScale.range();
      const yRange = this.yScale.range();
      this.xScale.range([xRange[0] + xDiff, xRange[1] + xDiff]);
      this.yScale.range([yRange[0] + yDiff, yRange[1] + yDiff]);
      this.screenmovestart = [this.xScale(this.mouseX), this.yScale(this.mouseY)];
    }
    if(this.clickedConnector) {
      this.lineend = [this.mouseX, this.mouseY];
    }
  }
  @HostListener('mouseup', ['$event'])
  mouseup(event: any) {
    if(event.button === 1) {
      this.screenmovestart = null;
    }
  }

  @HostListener('document:keydown', ['$event'])
  keydown(event: any) {
    if(event.code.startsWith('Shift')) {
      this.rotationMouseX = this.mouseX;
    }
    if(event.code.startsWith('Control')) {
      this.controlDown = true;
    }
  }

  @HostListener('document:keyup', ['$event'])
  keyup(event: any) {
    if(event.code.startsWith('Shift')) {
      this.rotationMouseX = null;
    }
    if(event.code.startsWith('Control')) {
      this.controlDown = false;
    }
  }

  sliderMoved(event, d) {
    this.setRelativeValue(d, event);
  }

  elementClicked(event, d) {
    if(this.controlDown && this.layoutType === 'spread') {
      this.flattenLayout(d, 2);
    }
    if(this.layoutType === 'flat') {
      this.spreadLayout();
      this.flattenLayout(d, 2);
    }
  }

  elementRightClicked(event, d) {
    if(d.type.name !== 'operator') {
      const dialogRef = this.dialog.open(EditVariableDialog, {
        data: <EditVariableDialogData>{
          symbol: d.symbol,
          name: d.name,
          description: d.description,
          type: d.type.name,
          value: d.value,
          minimum: d.minimum,
          maximum: d.maximum
        }
      });

      dialogRef.afterClosed().subscribe((result: EditVariableDialogData) => {
        if(result) {
          d.symbol = result.symbol;
          d.name = result.name;
          d.description = result.description;
          if(['constant', 'stock'].includes(d.type.name)) {
            d.value = result.value;
            d.minimum = result.minimum;
            d.maximum = result.maximum;
          }
        }
      });
    }
  }

  getConnectorPos(c) {
    const points = c.position(0,0,(c.data.type.boundingbox[2] / 2), (c.data.type.boundingbox[3] / 2));
    const rotated = this.rotatePoint(points, c.data.rotation);
    return [rotated[0] + c.data.x, rotated[1] + c.data.y, points[2] + c.data.rotation];
  }

  rotatePoint(point, degrees) {
    const x = point[0] * Math.cos(degrees) - point[1] * Math.sin(degrees);
    const y = point[1] * Math.cos(degrees) + point[0] * Math.sin(degrees);
    return [x, y];
  }

  connectorClicked(c, event) {
    event.stopPropagation();

    if(this.links.find(l => l.in === c || l.out === c)) return;

    if(!this.clickedConnector) {
      this.clickedConnector = c;
    } else {
      if(c.data === this.clickedConnector.data) return;
      
      if(this.clickedConnector.type === 'out' && c.type === 'in') {
        this.links.push({
          in: c,
          out: this.clickedConnector
        });
      } else if(this.clickedConnector.type === 'in' && c.type === 'out') {
        this.links.push({
          in: this.clickedConnector,
          out: c
        });
      } else {
        return;
      }
      this.lineend = null;
      this.clickedConnector = null;
    }
  }

  scaleAdjust(points: number[]) {
    const adjusted = points.slice();
    adjusted[0] = this.xScale(points[0]);
    adjusted[1] = this.yScale(points[1]);
    adjusted[2] = this.zScale(points[2]);
    return adjusted;
  }

  @HostListener('contextmenu', ['$event'])
  onRightClick(event) {
    event.preventDefault();
  }

  getMouseX(event) {
    return event.x - this.el.nativeElement.offsetLeft;
  }

  getMouseY(event) {
    return event.y - this.el.nativeElement.offsetTop;
  }

  toDegrees(radians: number) {
    return (radians / (2 * Math.PI)) * 360;
  }

  flattenLayout(centerElement: DataPoint, depth: number) {
    this.storedXScale = this.xScale;
    this.storedYScale = this.yScale;

    this.xScale = d3.scaleLinear()
    .domain([0, this.width])
    .range([0, this.width]);

    this.yScale = d3.scaleLinear()
    .domain([0, this.height])
    .range([0, this.height]);

    for(const d of this.unparkedData) {
      d.canvasPosition = {};
      d.canvasPosition.rotation = d.rotation;
      this.setPosition(d.canvasPosition, d.x, d.y);
      d.rotation = 0;
      d.hidden = true;
    }
    
    const root = this.createFlatNode(centerElement, depth + 1);

    this.setPosition(centerElement, this.width / 2, this.height / 2);
    centerElement.hidden = false;

    let outNodes = [root];
    for(let currentLevel = 1; currentLevel <= depth + 1; currentLevel++) {
      outNodes = flatMap(outNodes, n => n.out);
      let currentX = centerElement.x + (0.9 * centerElement.x * (currentLevel / depth));
      for(let i = 0; i < outNodes.length; i++) {
        this.setPosition(outNodes[i].element, currentX, (i + 1) * this.height / (outNodes.length + 1));
        outNodes[i].element.hidden = false;
      }
    }

    let inNodes = [root];
    for(let currentLevel = 1; currentLevel <= depth + 1; currentLevel++) {
      inNodes = flatMap(inNodes, n => n.in);
      let currentX = centerElement.x - (0.9 * centerElement.x * (currentLevel / depth));
      for(let i = 0; i < inNodes.length; i++) {
        this.setPosition(inNodes[i].element, currentX, (i + 1) * this.height / (inNodes.length + 1));
        inNodes[i].element.hidden = false;
      }
    }

    this.setForces();

    this.layoutType = 'flat';
  }

  setPosition(node, x: number, y: number) {
    node.x = x;
    node.y = y;
    node.fx = x;
    node.fy = y;
    node.anchorx = x;
    node.anchory = y;
  }

  createFlatNode(d: DataPoint, depth: number) {
    const flatNode = {
      element: d,
      in: [],
      out: []
    };

    if(depth === 0) return flatNode;

    for(const c of d.connectors) {
      const link = this.links.find(l => l[c.type] === c);
      if(link) {
        const connectedElement = link[this.oppositeType(c.type)].data;
        const newNode = this.createFlatNode(connectedElement, depth - 1);
        flatNode[c.type].push(newNode);
      }
    }

    return flatNode;
  }

  spreadLayout() {
    this.xScale = this.storedXScale;
    this.yScale = this.storedYScale;

    this.unparkedData.forEach(d => {
      Object.assign(d, d.canvasPosition);
      d.hidden = false;
    });

    this.setForces();

    this.layoutType = 'spread';
  }

  oppositeType(type: string) {
    return type === 'in' ? 'out' : 'in';
  }

  factorOffset(xFactor: number) {
    if(xFactor >= this.factorLevels[1]) return -80;
    if(xFactor >= this.factorLevels[0]) return -20;
    return 0;
  }

  factorScale(xFactor: number) {
    if(xFactor >= this.factorLevels[1]) return 'scale(0.25 0.25) ';
    if(xFactor >= this.factorLevels[0]) return 'scale(0.5 0.5) ';
    return '';
  }
}
