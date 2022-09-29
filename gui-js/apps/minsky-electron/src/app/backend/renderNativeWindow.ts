import {CppClass} from './backend';

class Colour
{
  r: number;
  b: number;
  g: number;
  a: number;
}

export class RenderNativeWindow extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  backgroundColour(...args: Colour[]): Colour {return this.callMethod("backgroundColour",...args);}
  renderToPS(name: string): void {this.callMethod("renderToPs",name);}
  renderToPDF(name: string): void {this.callMethod("renderToPDF",name);}
  renderToSVG(name: string): void {this.callMethod("renderToSVG",name);}
  renderToPNG(name: string): void {this.callMethod("renderToPNG",name);}
  renderToEMF(name: string): void {this.callMethod("renderToEMF",name);}
  requestRedraw(): void {this.callMethod("requestRedraw");}
}
