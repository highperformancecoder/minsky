import { Connector } from './connector';
import { Dimensions } from './dimensions';

export class ElementType {
    name: string;
    points: (x: number, y: number, width: number, height: number) => string;
    size: number[];
    connectors: Connector[];
    getDimensions: (length: number) => Dimensions;
}