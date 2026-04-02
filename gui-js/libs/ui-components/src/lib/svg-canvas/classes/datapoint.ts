import { ElementType } from './elementtype';
import { Connector } from './connector';
import { Dimensions } from './dimensions';

export class DataPoint {
    label: string;
    x?: number;
    y?: number;
    fx?: number;
    fy?: number;
    value: number;
    exponent: number;
    rotation: number;
    anchorx?: number;
    anchory?: number;
    type: ElementType;
    connectors: Connector[];
    canvasPosition;
    symbol;
    dimensions: Dimensions;
    hidden = false;
}