import { Dimensions } from '../classes/dimensions';

export class SvgVariableTypes {
    public static constant = {
        name: 'constant',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width*0.75},${y-height} ${x+width},${y} ${x+width*0.75},${y+height} ${x-width},${y+height}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-4 * lengthPart,-25,8 * lengthPart,50],
                labelbox: [-4 * lengthPart,-22.5,6 * lengthPart,45],
                valuebox: [lengthPart * 1,-20,lengthPart * 6,25],
                exponentbox: [lengthPart * 1,5,lengthPart * 6,25],
                valueline: [[-4 * lengthPart,-25],[3 * lengthPart,-25]]
            };
        },
        stroke: '#1c19d3',
        class: {'constant': true}
    };

    public static flow = {
        name: 'flow',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width*0.75},${y-height} ${x+width},${y} ${x+width*0.75},${y+height} ${x-width},${y+height} ${x-width*0.75},${y}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            },
            {
            type: 'in',
            position: (x: number, y: number, width: number, height: number) => [x-width*0.75,y,Math.PI]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-lengthPart * 4,-25,lengthPart * 8,50],
                labelbox: [-lengthPart * 4,-22.5,lengthPart * 6,45],
                valuebox: [lengthPart * 1,-20,lengthPart * 6,25],
                exponentbox: [lengthPart * 1,5,lengthPart * 6,25],
                valueline: [[-lengthPart * 4,-25],[lengthPart * 3,-25]]
            };
        },
        stroke: '#d32519',
        class: {'variable': true}
    };
    public static stock = {
        name: 'stock',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width*0.75},${y-height} ${x+width},${y} ${x+width*0.75},${y+height} ${x-width},${y+height}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            },
            {
            type: 'in',
            position: (x: number, y: number, width: number, height: number) => [x-width,y,Math.PI]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-lengthPart * 4,-25,lengthPart * 8,50],
                labelbox: [-lengthPart * 4,-22.5,lengthPart * 6,45],
                valuebox: [lengthPart * 1,-20,lengthPart * 6,25],
                exponentbox: [lengthPart * 1,5,lengthPart * 6,25],
                valueline: [[-lengthPart * 4,-25],[lengthPart * 3,-25]]
            };
        },
        stroke: '#d32519',
        class: {'variable': true}
    };
    public static integral = {
        name: 'integral',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width*0.75},${y-height} ${x+width},${y} ${x+width*0.75},${y+height} ${x-width},${y+height}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            },
            {
            type: 'in',
            position: (x: number, y: number, width: number, height: number) => [x-width,y,Math.PI]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-lengthPart * 4,-25,lengthPart * 8,50],
                labelbox: [-lengthPart * 4,-22.5,lengthPart * 6,45],
                valuebox: [lengthPart * 1,-20,lengthPart * 6,25],
                exponentbox: [lengthPart * 1,5,lengthPart * 6,25],
                valueline: [[-lengthPart * 4,-25],[lengthPart * 3,-25]]
            };
        },
        stroke: '#d32519',
        class: {'variable': true}
    }; 
    public static parameter = {
        name: 'parameter',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width*0.75},${y-height} ${x+width},${y} ${x+width*0.75},${y+height} ${x-width},${y+height}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-4 * lengthPart,-25,8 * lengthPart,50],
                labelbox: [-4 * lengthPart,-22.5,6 * lengthPart,45],
                valuebox: [lengthPart * 1,-20,lengthPart * 6,25],
                exponentbox: [lengthPart * 1,5,lengthPart * 6,25],
                valueline: [[-4 * lengthPart,-25],[3 * lengthPart,-25]]
            };
        },
        stroke: '#1c19d3',
        class: {'constant': true}
    };

    public static operator = {
        name: 'operator',
        points: (x: number, y: number, width: number, height: number) => {
            return `${x-width},${y-height} ${x+width},${y} ${x-width},${y+height}`;
        },
        connectors: [
            {
            type: 'out',
            position: (x: number, y: number, width: number, height: number) => [x+width,y,0]
            },
            {
            type: 'in',
            position: (x: number, y: number, width: number, height: number) => [x-width,y-height,Math.PI]
            },
            {
            type: 'in',
            position: (x: number, y: number, width: number, height: number) => [x-width,y+height,Math.PI]
            }
        ],
        getDimensions(length: number) {
            const lengthPart = length / 12;
            return {
                length: length,
                boundingbox: [-lengthPart * 2,-25,lengthPart * 4,50],
                labelbox: [-lengthPart * 2,-22.5,lengthPart * 2,45]
            };
        },
        stroke: '#19d3a2',
        class: {'greenthing': true}
    };
}

export class SvgCanvasHelper {
    static estimateVisibleLatex(latex) {
        let clean = latex;

        // 1. Remove spacing commands (e.g., \quad, \;, \,)
        clean = clean.replace(/\\[,;!\s]|\\quad|\\qquad/g, '');

        // 2. Remove formatting wrappers but keep their contents (e.g., \mathbf{x} -> x)
        clean = clean.replace(/\\(text|mathrm|mathbf|mathit|mathsf)\s*{([^}]*)}/g, '$2');

        // 3. Replace known single-character macros with a placeholder character (e.g., \alpha -> a)
        // This matches a backslash followed by letters
        clean = clean.replace(/\\[a-zA-Z]+/g, 'X');

        // 4. Remove structural characters that aren't printed (curly braces, alignment tabs)
        clean = clean.replace(/[{}_^&]/g, '');

        // 5. Remove whitespace
        clean = clean.replace(/\s+/g, '');

        return clean.length;
    }

    static createData(template) {
      const d = Object.assign({}, template);
      d.connectors = d.type.connectors.map(c => Object.assign({},c,{data: d}));
      d.actualLength = Math.max(100, SvgCanvasHelper.estimateVisibleLatex(template.symbol) * 15) + 120;
      d.dimensions = d.type.getDimensions(d.actualLength);
      return d;
    }
}