export class Connector {
    type: string;
    position: (x: number, y: number, width: number, height: number) => [number,number];
}