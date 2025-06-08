/*
This is a built file, please do not edit.
See RESTService/typescriptAPI for more information.
*/

import {CppClass, Sequence, Container, Map, Pair} from './backend';

class minsky__dummy {}
class minsky__EventInterface__KeyPressArgs {}
class minsky__GodleyIcon__MoveCellArgs {}
class minsky__RenderNativeWindow__RenderFrameArgs {}
class minsky__VariableType__TypeT {}
class CAPIRenderer  {}
class civita__ITensor__Args {}
class classdesc__json_pack_t {}
class classdesc__pack_t {}
class classdesc__RESTProcess_t {}
class ecolab__cairo__Surface {}

export class EventInterface extends CppClass {
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.item=new Item(this.$prefix()+'.item');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Item extends CppClass {
  bb: BoundingBox;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.$prefix()+'.bb');
  }
  async RESTProcess(a1: classdesc__RESTProcess_t,a2: string): Promise<void> {return this.$callMethod('RESTProcess',a1,a2);}
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async bookmarkId(): Promise<string> {return this.$callMethod('bookmarkId');}
  async bottom(): Promise<number> {return this.$callMethod('bottom');}
  async checkUnits(): Promise<object> {return this.$callMethod('checkUnits');}
  async classType(): Promise<string> {return this.$callMethod('classType');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clone(): Promise<Item> {return this.$callMethod('clone');}
  async closestInPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestInPort',a1,a2);}
  async closestOutPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestOutPort',a1,a2);}
  async contains(...args: any[]): Promise<boolean> {return this.$callMethod('contains',...args);}
  async corners(): Promise<object[]> {return this.$callMethod('corners');}
  async deleteAttachedWires(): Promise<void> {return this.$callMethod('deleteAttachedWires');}
  async deleteCallback(...args: string[]): Promise<string> {return this.$callMethod('deleteCallback',...args);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async disableDelayedTooltip(): Promise<void> {return this.$callMethod('disableDelayedTooltip');}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async displayTooltip(a1: minsky__dummy,a2: string): Promise<void> {return this.$callMethod('displayTooltip',a1,a2);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async drawPorts(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawPorts',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async drawSelected(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawSelected',a1);}
  async dummyDraw(): Promise<void> {return this.$callMethod('dummyDraw');}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async ensureBBValid(): Promise<void> {return this.$callMethod('ensureBBValid');}
  async flip(): Promise<void> {return this.$callMethod('flip');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async iHeight(...args: any[]): Promise<number> {return this.$callMethod('iHeight',...args);}
  async iWidth(...args: any[]): Promise<number> {return this.$callMethod('iWidth',...args);}
  async id(): Promise<string> {return this.$callMethod('id');}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async ioVar(): Promise<boolean> {return this.$callMethod('ioVar');}
  async itemPtrFromThis(): Promise<object> {return this.$callMethod('itemPtrFromThis');}
  async json_pack(a1: classdesc__json_pack_t): Promise<void> {return this.$callMethod('json_pack',a1);}
  async left(): Promise<number> {return this.$callMethod('left');}
  async m_sf(...args: number[]): Promise<number> {return this.$callMethod('m_sf',...args);}
  async m_x(...args: number[]): Promise<number> {return this.$callMethod('m_x',...args);}
  async m_y(...args: number[]): Promise<number> {return this.$callMethod('m_y',...args);}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async onBorder(...args: boolean[]): Promise<boolean> {return this.$callMethod('onBorder',...args);}
  async onItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onItem',a1,a2);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async onResizeHandles(...args: boolean[]): Promise<boolean> {return this.$callMethod('onResizeHandles',...args);}
  async portX(a1: number): Promise<number> {return this.$callMethod('portX',a1);}
  async portY(a1: number): Promise<number> {return this.$callMethod('portY',a1);}
  async ports(a1: number): Promise<object> {return this.$callMethod('ports',a1);}
  async portsSize(): Promise<number> {return this.$callMethod('portsSize');}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resizeHandleSize(): Promise<number> {return this.$callMethod('resizeHandleSize');}
  async right(): Promise<number> {return this.$callMethod('right');}
  async rotation(...args: any[]): Promise<number> {return this.$callMethod('rotation',...args);}
  async rotationAsRadians(): Promise<object> {return this.$callMethod('rotationAsRadians');}
  async scaleFactor(...args: any[]): Promise<number> {return this.$callMethod('scaleFactor',...args);}
  async select(a1: number,a2: number): Promise<object> {return this.$callMethod('select',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async throw_error(a1: string): Promise<void> {return this.$callMethod('throw_error',a1);}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async top(): Promise<number> {return this.$callMethod('top');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async updateIcon(a1: number): Promise<void> {return this.$callMethod('updateIcon',a1);}
  async value(): Promise<number> {return this.$callMethod('value');}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async x(): Promise<number> {return this.$callMethod('x');}
  async y(): Promise<number> {return this.$callMethod('y');}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class OperationBase extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
  }
  async addPorts(): Promise<void> {return this.$callMethod('addPorts');}
  async arg(...args: number[]): Promise<number> {return this.$callMethod('arg',...args);}
  async axis(...args: string[]): Promise<string> {return this.$callMethod('axis',...args);}
  async classify(a1: string): Promise<string> {return this.$callMethod('classify',a1);}
  async create(a1: string): Promise<OperationBase> {return this.$callMethod('create',a1);}
  async dimensions(): Promise<string[]> {return this.$callMethod('dimensions');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async drawUserFunction(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawUserFunction',a1);}
  async h(...args: number[]): Promise<number> {return this.$callMethod('h',...args);}
  async iconDraw(a1: minsky__dummy): Promise<void> {return this.$callMethod('iconDraw',a1);}
  async l(...args: number[]): Promise<number> {return this.$callMethod('l',...args);}
  async multiWire(): Promise<boolean> {return this.$callMethod('multiWire');}
  async numPorts(): Promise<number> {return this.$callMethod('numPorts');}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async operationCast(): Promise<OperationBase> {return this.$callMethod('operationCast');}
  async portValues(): Promise<string> {return this.$callMethod('portValues');}
  async r(...args: number[]): Promise<number> {return this.$callMethod('r',...args);}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resizeHandleCoords(): Promise<object> {return this.$callMethod('resizeHandleCoords');}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async type(): Promise<string> {return this.$callMethod('type');}
  async typeName(a1: number): Promise<string> {return this.$callMethod('typeName',a1);}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async value(): Promise<number> {return this.$callMethod('value');}
}

export class RenderNativeWindow extends EventInterface {
  backgroundColour: ecolab__cairo__Colour;
  constructor(prefix: string|EventInterface){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
  }
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
}

export class VariableBase extends Item {
  bb: BoundingBox;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.bb=new BoundingBox(this.$prefix()+'.bb');
  }
  async RESTProcess(a1: classdesc__RESTProcess_t,a2: string): Promise<void> {return this.$callMethod('RESTProcess',a1,a2);}
  async accessibleVars(): Promise<string[]> {return this.$callMethod('accessibleVars');}
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async adjustSliderBounds(): Promise<void> {return this.$callMethod('adjustSliderBounds');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async bookmarkId(): Promise<string> {return this.$callMethod('bookmarkId');}
  async bottom(): Promise<number> {return this.$callMethod('bottom');}
  async canonicalName(): Promise<string> {return this.$callMethod('canonicalName');}
  async checkUnits(): Promise<object> {return this.$callMethod('checkUnits');}
  async classType(): Promise<string> {return this.$callMethod('classType');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clone(): Promise<Item> {return this.$callMethod('clone');}
  async closestInPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestInPort',a1,a2);}
  async closestOutPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestOutPort',a1,a2);}
  async contains(...args: any[]): Promise<boolean> {return this.$callMethod('contains',...args);}
  async corners(): Promise<object[]> {return this.$callMethod('corners');}
  async create(a1: string): Promise<VariableBase> {return this.$callMethod('create',a1);}
  async defined(): Promise<boolean> {return this.$callMethod('defined');}
  async definition(): Promise<string> {return this.$callMethod('definition');}
  async deleteAttachedWires(): Promise<void> {return this.$callMethod('deleteAttachedWires');}
  async deleteCallback(...args: string[]): Promise<string> {return this.$callMethod('deleteCallback',...args);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async dimLabels(): Promise<string[]> {return this.$callMethod('dimLabels');}
  async dims(): Promise<number[]> {return this.$callMethod('dims');}
  async disableDelayedTooltip(): Promise<void> {return this.$callMethod('disableDelayedTooltip');}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async displayTooltip(a1: minsky__dummy,a2: string): Promise<void> {return this.$callMethod('displayTooltip',a1,a2);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async drawPorts(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawPorts',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async drawSelected(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawSelected',a1);}
  async dummyDraw(): Promise<void> {return this.$callMethod('dummyDraw');}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async enableSlider(...args: any[]): Promise<boolean> {return this.$callMethod('enableSlider',...args);}
  async engExp(): Promise<object> {return this.$callMethod('engExp');}
  async ensureBBValid(): Promise<void> {return this.$callMethod('ensureBBValid');}
  async ensureValueExists(a1: VariableValue,a2: string): Promise<void> {return this.$callMethod('ensureValueExists',a1,a2);}
  async exportAsCSV(a1: string,a2: boolean): Promise<void> {return this.$callMethod('exportAsCSV',a1,a2);}
  async flip(): Promise<void> {return this.$callMethod('flip');}
  async getDimLabelsPicked(): Promise<object> {return this.$callMethod('getDimLabelsPicked');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async iHeight(...args: any[]): Promise<number> {return this.$callMethod('iHeight',...args);}
  async iWidth(...args: any[]): Promise<number> {return this.$callMethod('iWidth',...args);}
  async id(): Promise<string> {return this.$callMethod('id');}
  async importFromCSV(a1: string[],a2: DataSpecSchema): Promise<void> {return this.$callMethod('importFromCSV',a1,a2);}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async init(...args: any[]): Promise<string> {return this.$callMethod('init',...args);}
  async inputWired(): Promise<boolean> {return this.$callMethod('inputWired');}
  async ioVar(): Promise<boolean> {return this.$callMethod('ioVar');}
  async isStock(): Promise<boolean> {return this.$callMethod('isStock');}
  async itemPtrFromThis(): Promise<object> {return this.$callMethod('itemPtrFromThis');}
  async json_pack(a1: classdesc__json_pack_t): Promise<void> {return this.$callMethod('json_pack',a1);}
  async left(): Promise<number> {return this.$callMethod('left');}
  async lhs(): Promise<boolean> {return this.$callMethod('lhs');}
  async local(): Promise<boolean> {return this.$callMethod('local');}
  async m_sf(...args: number[]): Promise<number> {return this.$callMethod('m_sf',...args);}
  async m_x(...args: number[]): Promise<number> {return this.$callMethod('m_x',...args);}
  async m_y(...args: number[]): Promise<number> {return this.$callMethod('m_y',...args);}
  async makeConsistentWithValue(): Promise<void> {return this.$callMethod('makeConsistentWithValue');}
  async mantissa(a1: EngNotation,a2: number): Promise<string> {return this.$callMethod('mantissa',a1,a2);}
  async miniPlotEnabled(...args: any[]): Promise<boolean> {return this.$callMethod('miniPlotEnabled',...args);}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async name(...args: any[]): Promise<string> {return this.$callMethod('name',...args);}
  async numPorts(): Promise<number> {return this.$callMethod('numPorts');}
  async onBorder(...args: boolean[]): Promise<boolean> {return this.$callMethod('onBorder',...args);}
  async onItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onItem',a1,a2);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async onResizeHandles(...args: boolean[]): Promise<boolean> {return this.$callMethod('onResizeHandles',...args);}
  async portX(a1: number): Promise<number> {return this.$callMethod('portX',a1);}
  async portY(a1: number): Promise<number> {return this.$callMethod('portY',a1);}
  async ports(a1: number): Promise<object> {return this.$callMethod('ports',a1);}
  async portsSize(): Promise<number> {return this.$callMethod('portsSize');}
  async rawName(): Promise<string> {return this.$callMethod('rawName');}
  async reloadCSV(): Promise<void> {return this.$callMethod('reloadCSV');}
  async resetMiniPlot(): Promise<void> {return this.$callMethod('resetMiniPlot');}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resizeHandleCoords(): Promise<object> {return this.$callMethod('resizeHandleCoords');}
  async resizeHandleSize(): Promise<number> {return this.$callMethod('resizeHandleSize');}
  async retype(a1: string): Promise<void> {return this.$callMethod('retype',a1);}
  async right(): Promise<number> {return this.$callMethod('right');}
  async rotation(...args: any[]): Promise<number> {return this.$callMethod('rotation',...args);}
  async rotationAsRadians(): Promise<object> {return this.$callMethod('rotationAsRadians');}
  async scaleFactor(...args: any[]): Promise<number> {return this.$callMethod('scaleFactor',...args);}
  async select(a1: number,a2: number): Promise<object> {return this.$callMethod('select',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async setDimLabelsPicked(a1: string,a2: string): Promise<object> {return this.$callMethod('setDimLabelsPicked',a1,a2);}
  async setUnits(a1: string): Promise<void> {return this.$callMethod('setUnits',a1);}
  async sliderMax(...args: any[]): Promise<number> {return this.$callMethod('sliderMax',...args);}
  async sliderMin(...args: any[]): Promise<number> {return this.$callMethod('sliderMin',...args);}
  async sliderStep(...args: any[]): Promise<number> {return this.$callMethod('sliderStep',...args);}
  async sliderStepRel(...args: any[]): Promise<boolean> {return this.$callMethod('sliderStepRel',...args);}
  async sliderVisible(): Promise<boolean> {return this.$callMethod('sliderVisible');}
  async temp(): Promise<boolean> {return this.$callMethod('temp');}
  async throw_error(a1: string): Promise<void> {return this.$callMethod('throw_error',a1);}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async toggleLocal(): Promise<void> {return this.$callMethod('toggleLocal');}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async top(): Promise<number> {return this.$callMethod('top');}
  async type(): Promise<string> {return this.$callMethod('type');}
  async typeName(a1: number): Promise<string> {return this.$callMethod('typeName',a1);}
  async units(...args: any[]): Promise<object> {return this.$callMethod('units',...args);}
  async unitsStr(): Promise<string> {return this.$callMethod('unitsStr');}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async updateIcon(a1: number): Promise<void> {return this.$callMethod('updateIcon',a1);}
  async vValue(): Promise<object> {return this.$callMethod('vValue');}
  async value(...args: any[]): Promise<number> {return this.$callMethod('value',...args);}
  async valueId(): Promise<string> {return this.$callMethod('valueId');}
  async valueIdInCurrentScope(a1: string): Promise<string> {return this.$callMethod('valueIdInCurrentScope',a1);}
  async variableCast(): Promise<VariableBase> {return this.$callMethod('variableCast');}
  async varsPassed(...args: number[]): Promise<number> {return this.$callMethod('varsPassed',...args);}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async x(): Promise<number> {return this.$callMethod('x');}
  async y(): Promise<number> {return this.$callMethod('y');}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

class minsky__Variable<T> extends VariableBase {}
export class Bookmark extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async name(...args: string[]): Promise<string> {return this.$callMethod('name',...args);}
  async x(...args: number[]): Promise<number> {return this.$callMethod('x',...args);}
  async y(...args: number[]): Promise<number> {return this.$callMethod('y',...args);}
  async zoom(...args: number[]): Promise<number> {return this.$callMethod('zoom',...args);}
}

export class BoundingBox extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async bottom(): Promise<number> {return this.$callMethod('bottom');}
  async contains(a1: number,a2: number): Promise<boolean> {return this.$callMethod('contains',a1,a2);}
  async height(): Promise<number> {return this.$callMethod('height');}
  async left(): Promise<number> {return this.$callMethod('left');}
  async right(): Promise<number> {return this.$callMethod('right');}
  async top(): Promise<number> {return this.$callMethod('top');}
  async update(a1: Item): Promise<void> {return this.$callMethod('update',a1);}
  async valid(): Promise<boolean> {return this.$callMethod('valid');}
  async width(): Promise<number> {return this.$callMethod('width');}
}

export class CSVDialog extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  spec: DataSpec;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.item=new Item(this.$prefix()+'.item');
    this.spec=new DataSpec(this.$prefix()+'.spec');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async classifyColumns(): Promise<void> {return this.$callMethod('classifyColumns');}
  async colWidth(...args: number[]): Promise<number> {return this.$callMethod('colWidth',...args);}
  async columnOver(a1: number): Promise<number> {return this.$callMethod('columnOver',a1);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async correctedUniqueValues(): Promise<number[]> {return this.$callMethod('correctedUniqueValues');}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async flashNameRow(...args: boolean[]): Promise<boolean> {return this.$callMethod('flashNameRow',...args);}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async guessSpecAndLoadFile(): Promise<void> {return this.$callMethod('guessSpecAndLoadFile');}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async loadFile(): Promise<void> {return this.$callMethod('loadFile');}
  async loadFileFromName(a1: string): Promise<void> {return this.$callMethod('loadFileFromName',a1);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async numInitialLines(...args: number[]): Promise<number> {return this.$callMethod('numInitialLines',...args);}
  async parseLines(a1: number): Promise<string[][]> {return this.$callMethod('parseLines',a1);}
  async populateHeader(a1: number): Promise<void> {return this.$callMethod('populateHeader',a1);}
  async populateHeaders(): Promise<void> {return this.$callMethod('populateHeaders');}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async reportFromFile(a1: string,a2: string): Promise<void> {return this.$callMethod('reportFromFile',a1,a2);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async rowOver(a1: number): Promise<number> {return this.$callMethod('rowOver',a1);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async tableWidth(): Promise<number> {return this.$callMethod('tableWidth');}
  async url(...args: string[]): Promise<string> {return this.$callMethod('url',...args);}
  async xoffs(...args: number[]): Promise<number> {return this.$callMethod('xoffs',...args);}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Canvas extends RenderNativeWindow {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  itemFocus: Item;
  itemIndicator: Item;
  lasso: LassoBox;
  model: Group;
  selection: Selection;
  updateRegion: LassoBox;
  wire: Wire;
  wireFocus: Wire;
  constructor(prefix: string|RenderNativeWindow){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.item=new Item(this.$prefix()+'.item');
    this.itemFocus=new Item(this.$prefix()+'.itemFocus');
    this.itemIndicator=new Item(this.$prefix()+'.itemIndicator');
    this.lasso=new LassoBox(this.$prefix()+'.lasso');
    this.model=new Group(this.$prefix()+'.model');
    this.selection=new Selection(this.$prefix()+'.selection');
    this.updateRegion=new LassoBox(this.$prefix()+'.updateRegion');
    this.wire=new Wire(this.$prefix()+'.wire');
    this.wireFocus=new Wire(this.$prefix()+'.wireFocus');
  }
  async addGodley(): Promise<void> {return this.$callMethod('addGodley');}
  async addGroup(): Promise<void> {return this.$callMethod('addGroup');}
  async addLock(): Promise<void> {return this.$callMethod('addLock');}
  async addNote(a1: string): Promise<void> {return this.$callMethod('addNote',a1);}
  async addOperation(a1: string): Promise<void> {return this.$callMethod('addOperation',a1);}
  async addPlot(): Promise<void> {return this.$callMethod('addPlot');}
  async addRavel(): Promise<void> {return this.$callMethod('addRavel');}
  async addSheet(): Promise<void> {return this.$callMethod('addSheet');}
  async addSwitch(): Promise<void> {return this.$callMethod('addSwitch');}
  async addVariable(a1: string,a2: string): Promise<void> {return this.$callMethod('addVariable',a1,a2);}
  async applyDefaultPlotOptions(): Promise<void> {return this.$callMethod('applyDefaultPlotOptions');}
  async clickType(...args: string[]): Promise<string> {return this.$callMethod('clickType',...args);}
  async closestInPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestInPort',a1,a2);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async copyAllFlowVars(): Promise<void> {return this.$callMethod('copyAllFlowVars');}
  async copyAllStockVars(): Promise<void> {return this.$callMethod('copyAllStockVars');}
  async copyItem(): Promise<void> {return this.$callMethod('copyItem');}
  async defaultRotation(...args: number[]): Promise<number> {return this.$callMethod('defaultRotation',...args);}
  async delHandle(a1: number,a2: number): Promise<void> {return this.$callMethod('delHandle',a1,a2);}
  async deleteItem(): Promise<void> {return this.$callMethod('deleteItem');}
  async deleteWire(): Promise<void> {return this.$callMethod('deleteWire');}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async findVariableDefinition(): Promise<boolean> {return this.$callMethod('findVariableDefinition');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async groupSelection(): Promise<void> {return this.$callMethod('groupSelection');}
  async handleSelected(...args: number[]): Promise<number> {return this.$callMethod('handleSelected',...args);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async itemAt(a1: number,a2: number): Promise<object> {return this.$callMethod('itemAt',a1,a2);}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async lassoMode(...args: string[]): Promise<string> {return this.$callMethod('lassoMode',...args);}
  async lockRavelsInSelection(): Promise<void> {return this.$callMethod('lockRavelsInSelection');}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveOffsX(...args: number[]): Promise<number> {return this.$callMethod('moveOffsX',...args);}
  async moveOffsY(...args: number[]): Promise<number> {return this.$callMethod('moveOffsY',...args);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async openGroupInCanvas(a1: Item): Promise<void> {return this.$callMethod('openGroupInCanvas',a1);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async ravelsSelected(): Promise<number> {return this.$callMethod('ravelsSelected');}
  async recentre(): Promise<void> {return this.$callMethod('recentre');}
  async redraw(): Promise<boolean> {return this.$callMethod('redraw');}
  async redrawAll(...args: boolean[]): Promise<boolean> {return this.$callMethod('redrawAll',...args);}
  async redrawRequested(): Promise<boolean> {return this.$callMethod('redrawRequested');}
  async redrawUpdateRegion(): Promise<boolean> {return this.$callMethod('redrawUpdateRegion');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async removeItemFromItsGroup(): Promise<void> {return this.$callMethod('removeItemFromItsGroup');}
  async renameAllInstances(a1: string): Promise<void> {return this.$callMethod('renameAllInstances',a1);}
  async renameItem(a1: string): Promise<void> {return this.$callMethod('renameItem',a1);}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPNGCropped(a1: string,a2: minsky__Canvas__ZoomCrop): Promise<void> {return this.$callMethod('renderToPNGCropped',a1,a2);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async rotateItem(a1: number,a2: number): Promise<void> {return this.$callMethod('rotateItem',a1,a2);}
  async rotatingItem(...args: boolean[]): Promise<boolean> {return this.$callMethod('rotatingItem',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async select(a1: LassoBox): Promise<void> {return this.$callMethod('select',a1);}
  async selectAllVariables(): Promise<void> {return this.$callMethod('selectAllVariables');}
  async selectVar(a1: number,a2: number): Promise<boolean> {return this.$callMethod('selectVar',a1,a2);}
  async setDefaultPlotOptions(): Promise<void> {return this.$callMethod('setDefaultPlotOptions');}
  async setItemFocus(a1: Item): Promise<void> {return this.$callMethod('setItemFocus',a1);}
  async setItemFromItemFocus(): Promise<void> {return this.$callMethod('setItemFromItemFocus');}
  async surface(): Promise<ecolab__cairo__Surface> {return this.$callMethod('surface');}
  async termX(...args: number[]): Promise<number> {return this.$callMethod('termX',...args);}
  async termY(...args: number[]): Promise<number> {return this.$callMethod('termY',...args);}
  async ungroupItem(): Promise<void> {return this.$callMethod('ungroupItem');}
  async unlockRavelsInSelection(): Promise<void> {return this.$callMethod('unlockRavelsInSelection');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
  async zoomToDisplay(): Promise<void> {return this.$callMethod('zoomToDisplay');}
  async zoomToFit(): Promise<void> {return this.$callMethod('zoomToFit');}
}

export class DataOp extends Item {
  data: Map<number,number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.data=new Map<number,number>(this.$prefix()+'.data');
  }
  async deriv(a1: number): Promise<number> {return this.$callMethod('deriv',a1);}
  async description(...args: any[]): Promise<string> {return this.$callMethod('description',...args);}
  async initRandom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('initRandom',a1,a2,a3);}
  async interpolate(a1: number): Promise<number> {return this.$callMethod('interpolate',a1);}
  async pack(a1: classdesc__pack_t,a2: string): Promise<void> {return this.$callMethod('pack',a1,a2);}
  async readData(a1: string): Promise<void> {return this.$callMethod('readData',a1);}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async unpack(a1: classdesc__pack_t,a2: string): Promise<void> {return this.$callMethod('unpack',a1,a2);}
}

export class DataSpec extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensionNames: Sequence<string>;
  dimensions: Sequence<civita__Dimension>;
  horizontalDimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(this.$prefix()+'.dataCols');
    this.dimensionCols=new Container<number>(this.$prefix()+'.dimensionCols');
    this.dimensionNames=new Sequence<string>(this.$prefix()+'.dimensionNames');
    this.dimensions=new Sequence<civita__Dimension>(this.$prefix()+'.dimensions',civita__Dimension);
    this.horizontalDimension=new civita__Dimension(this.$prefix()+'.horizontalDimension');
  }
  async counter(...args: boolean[]): Promise<boolean> {return this.$callMethod('counter',...args);}
  async dataColOffset(...args: number[]): Promise<number> {return this.$callMethod('dataColOffset',...args);}
  async dataRowOffset(...args: number[]): Promise<number> {return this.$callMethod('dataRowOffset',...args);}
  async decSeparator(...args: number[]): Promise<number> {return this.$callMethod('decSeparator',...args);}
  async dontFail(...args: boolean[]): Promise<boolean> {return this.$callMethod('dontFail',...args);}
  async duplicateKeyAction(...args: string[]): Promise<string> {return this.$callMethod('duplicateKeyAction',...args);}
  async escape(...args: number[]): Promise<number> {return this.$callMethod('escape',...args);}
  async guessFromFile(a1: string): Promise<void> {return this.$callMethod('guessFromFile',a1);}
  async guessFromStream(a1: minsky__dummy,a2: number): Promise<void> {return this.$callMethod('guessFromStream',a1,a2);}
  async headerRow(...args: number[]): Promise<number> {return this.$callMethod('headerRow',...args);}
  async horizontalDimName(...args: string[]): Promise<string> {return this.$callMethod('horizontalDimName',...args);}
  async maxColumn(...args: number[]): Promise<number> {return this.$callMethod('maxColumn',...args);}
  async mergeDelimiters(...args: boolean[]): Promise<boolean> {return this.$callMethod('mergeDelimiters',...args);}
  async missingValue(...args: number[]): Promise<number> {return this.$callMethod('missingValue',...args);}
  async nColAxes(): Promise<number> {return this.$callMethod('nColAxes');}
  async nRowAxes(): Promise<number> {return this.$callMethod('nRowAxes');}
  async numCols(...args: number[]): Promise<number> {return this.$callMethod('numCols',...args);}
  async populateFromRavelMetadata(a1: string,a2: string,a3: number): Promise<void> {return this.$callMethod('populateFromRavelMetadata',a1,a2,a3);}
  async quote(...args: number[]): Promise<number> {return this.$callMethod('quote',...args);}
  async separator(...args: number[]): Promise<number> {return this.$callMethod('separator',...args);}
  async setDataArea(a1: number,a2: number): Promise<void> {return this.$callMethod('setDataArea',a1,a2);}
  async toSchema(): Promise<object> {return this.$callMethod('toSchema');}
  async toggleDimension(a1: number): Promise<void> {return this.$callMethod('toggleDimension',a1);}
  async uniqueValues(): Promise<number[]> {return this.$callMethod('uniqueValues');}
}

export class DataSpecSchema extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensionNames: Sequence<string>;
  dimensions: Sequence<civita__Dimension>;
  horizontalDimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(this.$prefix()+'.dataCols');
    this.dimensionCols=new Container<number>(this.$prefix()+'.dimensionCols');
    this.dimensionNames=new Sequence<string>(this.$prefix()+'.dimensionNames');
    this.dimensions=new Sequence<civita__Dimension>(this.$prefix()+'.dimensions',civita__Dimension);
    this.horizontalDimension=new civita__Dimension(this.$prefix()+'.horizontalDimension');
  }
  async counter(...args: boolean[]): Promise<boolean> {return this.$callMethod('counter',...args);}
  async dataColOffset(...args: number[]): Promise<number> {return this.$callMethod('dataColOffset',...args);}
  async dataRowOffset(...args: number[]): Promise<number> {return this.$callMethod('dataRowOffset',...args);}
  async decSeparator(...args: number[]): Promise<number> {return this.$callMethod('decSeparator',...args);}
  async dontFail(...args: boolean[]): Promise<boolean> {return this.$callMethod('dontFail',...args);}
  async duplicateKeyAction(...args: string[]): Promise<string> {return this.$callMethod('duplicateKeyAction',...args);}
  async escape(...args: number[]): Promise<number> {return this.$callMethod('escape',...args);}
  async headerRow(...args: number[]): Promise<number> {return this.$callMethod('headerRow',...args);}
  async horizontalDimName(...args: string[]): Promise<string> {return this.$callMethod('horizontalDimName',...args);}
  async mergeDelimiters(...args: boolean[]): Promise<boolean> {return this.$callMethod('mergeDelimiters',...args);}
  async missingValue(...args: number[]): Promise<number> {return this.$callMethod('missingValue',...args);}
  async numCols(...args: number[]): Promise<number> {return this.$callMethod('numCols',...args);}
  async quote(...args: number[]): Promise<number> {return this.$callMethod('quote',...args);}
  async separator(...args: number[]): Promise<number> {return this.$callMethod('separator',...args);}
}

export class EngNotation extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async engExp(...args: number[]): Promise<number> {return this.$callMethod('engExp',...args);}
  async sciExp(...args: number[]): Promise<number> {return this.$callMethod('sciExp',...args);}
}

export class EquationDisplay extends RenderNativeWindow {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string|RenderNativeWindow){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.item=new Item(this.$prefix()+'.item');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async m_zoomFactor(...args: number[]): Promise<number> {return this.$callMethod('m_zoomFactor',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async offsx(...args: number[]): Promise<number> {return this.$callMethod('offsx',...args);}
  async offsy(...args: number[]): Promise<number> {return this.$callMethod('offsy',...args);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class EvalGodley extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async compatibility(...args: boolean[]): Promise<boolean> {return this.$callMethod('compatibility',...args);}
  async eval(a1: number,a2: number): Promise<void> {return this.$callMethod('eval',a1,a2);}
}

export class FontDisplay extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.item=new Item(this.$prefix()+'.item');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class GodleyIcon extends Item {
  editor: GodleyTableEditor;
  popup: GodleyTableWindow;
  svgRenderer: SVGRenderer;
  table: GodleyTable;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.editor=new GodleyTableEditor(this.$prefix()+'.editor');
    this.popup=new GodleyTableWindow(this.$prefix()+'.popup');
    this.svgRenderer=new SVGRenderer(this.$prefix()+'.svgRenderer');
    this.table=new GodleyTable(this.$prefix()+'.table');
  }
  async adjustPopupWidgets(): Promise<void> {return this.$callMethod('adjustPopupWidgets');}
  async bottomMargin(): Promise<number> {return this.$callMethod('bottomMargin');}
  async buttonDisplay(): Promise<boolean> {return this.$callMethod('buttonDisplay');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clone(): Promise<GodleyIcon> {return this.$callMethod('clone');}
  async currency(...args: string[]): Promise<string> {return this.$callMethod('currency',...args);}
  async deleteRow(a1: number): Promise<void> {return this.$callMethod('deleteRow',a1);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async flowSignature(a1: number): Promise<object> {return this.$callMethod('flowSignature',a1);}
  async flowVars(): Promise<Sequence<VariableBase>> {return this.$callMethod('flowVars');}
  async godleyIconCast(): Promise<GodleyIcon> {return this.$callMethod('godleyIconCast');}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async insertControlled(a1: Selection): Promise<void> {return this.$callMethod('insertControlled',a1);}
  async leftMargin(): Promise<number> {return this.$callMethod('leftMargin');}
  async moveCell(a1: minsky__GodleyIcon__MoveCellArgs): Promise<void> {return this.$callMethod('moveCell',a1);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async removeControlledItems(a1: GroupItems): Promise<void> {return this.$callMethod('removeControlledItems',a1);}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async rowSum(a1: number): Promise<string> {return this.$callMethod('rowSum',a1);}
  async scaleIcon(a1: number,a2: number): Promise<void> {return this.$callMethod('scaleIcon',a1,a2);}
  async select(a1: number,a2: number): Promise<object> {return this.$callMethod('select',a1,a2);}
  async setCell(a1: number,a2: number,a3: string): Promise<void> {return this.$callMethod('setCell',a1,a2,a3);}
  async setCurrency(a1: string): Promise<void> {return this.$callMethod('setCurrency',a1);}
  async stockVarUnits(a1: string,a2: boolean): Promise<object> {return this.$callMethod('stockVarUnits',a1,a2);}
  async stockVars(): Promise<Sequence<VariableBase>> {return this.$callMethod('stockVars');}
  async summarise(): Promise<object[]> {return this.$callMethod('summarise');}
  async toggleButtons(): Promise<void> {return this.$callMethod('toggleButtons');}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async toggleVariableDisplay(): Promise<void> {return this.$callMethod('toggleVariableDisplay');}
  async update(): Promise<void> {return this.$callMethod('update');}
  async valueId(a1: string): Promise<string> {return this.$callMethod('valueId',a1);}
  async variableDisplay(): Promise<boolean> {return this.$callMethod('variableDisplay');}
  async wiresAttached(): Promise<boolean> {return this.$callMethod('wiresAttached');}
}

export class GodleyTable extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async assetClass(...args: any[]): Promise<Sequence<string>> {return this.$callMethod('assetClass',...args);}
  async balanceEquity(a1: number): Promise<void> {return this.$callMethod('balanceEquity',a1);}
  async cell(a1: number,a2: number): Promise<string> {return this.$callMethod('cell',a1,a2);}
  async cellInTable(a1: number,a2: number): Promise<boolean> {return this.$callMethod('cellInTable',a1,a2);}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async cols(): Promise<number> {return this.$callMethod('cols');}
  async deleteCol(a1: number): Promise<void> {return this.$callMethod('deleteCol',a1);}
  async deleteRow(a1: number): Promise<void> {return this.$callMethod('deleteRow',a1);}
  async dimension(a1: number,a2: number): Promise<void> {return this.$callMethod('dimension',a1,a2);}
  async doubleEntryCompliant(...args: boolean[]): Promise<boolean> {return this.$callMethod('doubleEntryCompliant',...args);}
  async exportToCSV(a1: string): Promise<void> {return this.$callMethod('exportToCSV',a1);}
  async exportToLaTeX(a1: string): Promise<void> {return this.$callMethod('exportToLaTeX',a1);}
  async getCell(a1: number,a2: number): Promise<string> {return this.$callMethod('getCell',a1,a2);}
  async getColumn(a1: number): Promise<string[]> {return this.$callMethod('getColumn',a1);}
  async getColumnVariables(): Promise<string[]> {return this.$callMethod('getColumnVariables');}
  async getData(): Promise<Sequence<string[]>> {return this.$callMethod('getData');}
  async getVariables(): Promise<string[]> {return this.$callMethod('getVariables');}
  async initialConditionRow(a1: number): Promise<boolean> {return this.$callMethod('initialConditionRow',a1);}
  async initialConditions(...args: string[]): Promise<string> {return this.$callMethod('initialConditions',...args);}
  async insertCol(a1: number): Promise<void> {return this.$callMethod('insertCol',a1);}
  async insertRow(a1: number): Promise<void> {return this.$callMethod('insertRow',a1);}
  async moveCol(a1: number,a2: number): Promise<void> {return this.$callMethod('moveCol',a1,a2);}
  async moveRow(a1: number,a2: number): Promise<void> {return this.$callMethod('moveRow',a1,a2);}
  async nameUnique(): Promise<void> {return this.$callMethod('nameUnique');}
  async orderAssetClasses(): Promise<void> {return this.$callMethod('orderAssetClasses');}
  async rename(a1: string,a2: string): Promise<void> {return this.$callMethod('rename',a1,a2);}
  async renameFlows(a1: string,a2: string): Promise<void> {return this.$callMethod('renameFlows',a1,a2);}
  async renameStock(a1: string,a2: string): Promise<void> {return this.$callMethod('renameStock',a1,a2);}
  async resize(a1: number,a2: number): Promise<void> {return this.$callMethod('resize',a1,a2);}
  async rowSum(a1: number): Promise<string> {return this.$callMethod('rowSum',a1);}
  async rowSumAsMap(a1: number): Promise<object> {return this.$callMethod('rowSumAsMap',a1);}
  async rows(): Promise<number> {return this.$callMethod('rows');}
  async savedText(...args: string[]): Promise<string> {return this.$callMethod('savedText',...args);}
  async setCell(a1: number,a2: number,a3: string): Promise<void> {return this.$callMethod('setCell',a1,a2,a3);}
  async setDEmode(a1: boolean): Promise<void> {return this.$callMethod('setDEmode',a1);}
  async signConventionReversed(a1: number): Promise<boolean> {return this.$callMethod('signConventionReversed',a1);}
  async singleEquity(): Promise<boolean> {return this.$callMethod('singleEquity');}
  async singularRow(a1: number,a2: number): Promise<boolean> {return this.$callMethod('singularRow',a1,a2);}
  async stringify(a1: Map<string,number>): Promise<string> {return this.$callMethod('stringify',a1);}
  async title(...args: string[]): Promise<string> {return this.$callMethod('title',...args);}
}

export class GodleyTableEditor extends CppClass {
  colLeftMargin: Sequence<number>;
  constructor(prefix: string){
    super(prefix);
    this.colLeftMargin=new Sequence<number>(this.$prefix()+'.colLeftMargin');
  }
  async addFlow(a1: number): Promise<void> {return this.$callMethod('addFlow',a1);}
  async addFlowByRow(a1: number): Promise<void> {return this.$callMethod('addFlowByRow',a1);}
  async addStockVar(a1: number): Promise<void> {return this.$callMethod('addStockVar',a1);}
  async addStockVarByCol(a1: number): Promise<void> {return this.$callMethod('addStockVarByCol',a1);}
  async adjustWidgets(): Promise<void> {return this.$callMethod('adjustWidgets');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clickTypeZoomed(a1: number,a2: number): Promise<string> {return this.$callMethod('clickTypeZoomed',a1,a2);}
  async colXZoomed(a1: number): Promise<number> {return this.$callMethod('colXZoomed',a1);}
  async columnButtonsOffset(...args: number[]): Promise<number> {return this.$callMethod('columnButtonsOffset',...args);}
  async copy(): Promise<void> {return this.$callMethod('copy');}
  async cut(): Promise<void> {return this.$callMethod('cut');}
  async delSelection(): Promise<void> {return this.$callMethod('delSelection');}
  async deleteFlow(a1: number): Promise<void> {return this.$callMethod('deleteFlow',a1);}
  async deleteFlowByRow(a1: number): Promise<void> {return this.$callMethod('deleteFlowByRow',a1);}
  async deleteStockVar(a1: number): Promise<void> {return this.$callMethod('deleteStockVar',a1);}
  async deleteStockVarByCol(a1: number): Promise<void> {return this.$callMethod('deleteStockVarByCol',a1);}
  async disableButtons(): Promise<void> {return this.$callMethod('disableButtons');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async drawButtons(...args: boolean[]): Promise<boolean> {return this.$callMethod('drawButtons',...args);}
  async enableButtons(): Promise<void> {return this.$callMethod('enableButtons');}
  async godleyIcon(): Promise<GodleyIcon> {return this.$callMethod('godleyIcon');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async highlightCell(a1: minsky__dummy,a2: number,a3: number): Promise<void> {return this.$callMethod('highlightCell',a1,a2,a3);}
  async highlightColumn(a1: minsky__dummy,a2: number): Promise<void> {return this.$callMethod('highlightColumn',a1,a2);}
  async highlightRow(a1: minsky__dummy,a2: number): Promise<void> {return this.$callMethod('highlightRow',a1,a2);}
  async historyPtr(...args: number[]): Promise<number> {return this.$callMethod('historyPtr',...args);}
  async hoverCol(...args: number[]): Promise<number> {return this.$callMethod('hoverCol',...args);}
  async hoverRow(...args: number[]): Promise<number> {return this.$callMethod('hoverRow',...args);}
  async importStockVar(a1: string,a2: number): Promise<void> {return this.$callMethod('importStockVar',a1,a2);}
  async importStockVarByCol(a1: string,a2: number): Promise<void> {return this.$callMethod('importStockVarByCol',a1,a2);}
  async insertIdx(...args: number[]): Promise<number> {return this.$callMethod('insertIdx',...args);}
  async keyPress(a1: number,a2: string): Promise<void> {return this.$callMethod('keyPress',a1,a2);}
  async leftTableOffset(...args: number[]): Promise<number> {return this.$callMethod('leftTableOffset',...args);}
  async matchingTableColumns(a1: number): Promise<string[]> {return this.$callMethod('matchingTableColumns',a1);}
  async matchingTableColumnsByCol(a1: number): Promise<string[]> {return this.$callMethod('matchingTableColumnsByCol',a1);}
  async maxHistory(...args: number[]): Promise<number> {return this.$callMethod('maxHistory',...args);}
  async minColumnWidth(...args: number[]): Promise<number> {return this.$callMethod('minColumnWidth',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseMoveB1(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMoveB1',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveAssetClass(a1: number,a2: number): Promise<string> {return this.$callMethod('moveAssetClass',a1,a2);}
  async navigateDown(): Promise<void> {return this.$callMethod('navigateDown');}
  async navigateLeft(): Promise<void> {return this.$callMethod('navigateLeft');}
  async navigateRight(): Promise<void> {return this.$callMethod('navigateRight');}
  async navigateUp(): Promise<void> {return this.$callMethod('navigateUp');}
  async paste(): Promise<void> {return this.$callMethod('paste');}
  async pulldownHot(...args: number[]): Promise<number> {return this.$callMethod('pulldownHot',...args);}
  async pushHistory(): Promise<void> {return this.$callMethod('pushHistory');}
  async rowHeight(...args: number[]): Promise<number> {return this.$callMethod('rowHeight',...args);}
  async rowYZoomed(a1: number): Promise<number> {return this.$callMethod('rowYZoomed',a1);}
  async scrollColStart(...args: number[]): Promise<number> {return this.$callMethod('scrollColStart',...args);}
  async scrollRowStart(...args: number[]): Promise<number> {return this.$callMethod('scrollRowStart',...args);}
  async selectIdx(...args: number[]): Promise<number> {return this.$callMethod('selectIdx',...args);}
  async selectedCellInTable(): Promise<boolean> {return this.$callMethod('selectedCellInTable');}
  async selectedCol(...args: number[]): Promise<number> {return this.$callMethod('selectedCol',...args);}
  async selectedRow(...args: number[]): Promise<number> {return this.$callMethod('selectedRow',...args);}
  async srcCol(...args: number[]): Promise<number> {return this.$callMethod('srcCol',...args);}
  async srcRow(...args: number[]): Promise<number> {return this.$callMethod('srcRow',...args);}
  async swapAssetClass(a1: number,a2: number): Promise<string> {return this.$callMethod('swapAssetClass',a1,a2);}
  async textIdx(a1: number): Promise<number> {return this.$callMethod('textIdx',a1);}
  async topTableOffset(...args: number[]): Promise<number> {return this.$callMethod('topTableOffset',...args);}
  async undo(a1: number): Promise<void> {return this.$callMethod('undo',a1);}
  async update(): Promise<void> {return this.$callMethod('update');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async zoomFactor(...args: number[]): Promise<number> {return this.$callMethod('zoomFactor',...args);}
}

export class GodleyTableWindow extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  colLeftMargin: Sequence<number>;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.colLeftMargin=new Sequence<number>(this.$prefix()+'.colLeftMargin');
    this.item=new Item(this.$prefix()+'.item');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async addFlow(a1: number): Promise<void> {return this.$callMethod('addFlow',a1);}
  async addFlowByRow(a1: number): Promise<void> {return this.$callMethod('addFlowByRow',a1);}
  async addStockVar(a1: number): Promise<void> {return this.$callMethod('addStockVar',a1);}
  async addStockVarByCol(a1: number): Promise<void> {return this.$callMethod('addStockVarByCol',a1);}
  async adjustWidgets(): Promise<void> {return this.$callMethod('adjustWidgets');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clickTypeZoomed(a1: number,a2: number): Promise<string> {return this.$callMethod('clickTypeZoomed',a1,a2);}
  async colXZoomed(a1: number): Promise<number> {return this.$callMethod('colXZoomed',a1);}
  async columnButtonsOffset(...args: number[]): Promise<number> {return this.$callMethod('columnButtonsOffset',...args);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async copy(): Promise<void> {return this.$callMethod('copy');}
  async cut(): Promise<void> {return this.$callMethod('cut');}
  async delSelection(): Promise<void> {return this.$callMethod('delSelection');}
  async deleteFlow(a1: number): Promise<void> {return this.$callMethod('deleteFlow',a1);}
  async deleteFlowByRow(a1: number): Promise<void> {return this.$callMethod('deleteFlowByRow',a1);}
  async deleteStockVar(a1: number): Promise<void> {return this.$callMethod('deleteStockVar',a1);}
  async deleteStockVarByCol(a1: number): Promise<void> {return this.$callMethod('deleteStockVarByCol',a1);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async disableButtons(): Promise<void> {return this.$callMethod('disableButtons');}
  async draw(...args: any[]): Promise<void> {return this.$callMethod('draw',...args);}
  async drawButtons(...args: boolean[]): Promise<boolean> {return this.$callMethod('drawButtons',...args);}
  async enableButtons(): Promise<void> {return this.$callMethod('enableButtons');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async godleyIcon(): Promise<GodleyIcon> {return this.$callMethod('godleyIcon');}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async highlightCell(a1: minsky__dummy,a2: number,a3: number): Promise<void> {return this.$callMethod('highlightCell',a1,a2,a3);}
  async highlightColumn(a1: minsky__dummy,a2: number): Promise<void> {return this.$callMethod('highlightColumn',a1,a2);}
  async highlightRow(a1: minsky__dummy,a2: number): Promise<void> {return this.$callMethod('highlightRow',a1,a2);}
  async historyPtr(...args: number[]): Promise<number> {return this.$callMethod('historyPtr',...args);}
  async hoverCol(...args: number[]): Promise<number> {return this.$callMethod('hoverCol',...args);}
  async hoverRow(...args: number[]): Promise<number> {return this.$callMethod('hoverRow',...args);}
  async importStockVar(a1: string,a2: number): Promise<void> {return this.$callMethod('importStockVar',a1,a2);}
  async importStockVarByCol(a1: string,a2: number): Promise<void> {return this.$callMethod('importStockVarByCol',a1,a2);}
  async init(): Promise<void> {return this.$callMethod('init');}
  async insertIdx(...args: number[]): Promise<number> {return this.$callMethod('insertIdx',...args);}
  async keyPress(...args: any[]): Promise<boolean> {return this.$callMethod('keyPress',...args);}
  async leftTableOffset(...args: number[]): Promise<number> {return this.$callMethod('leftTableOffset',...args);}
  async matchingTableColumns(a1: number): Promise<string[]> {return this.$callMethod('matchingTableColumns',a1);}
  async matchingTableColumnsByCol(a1: number): Promise<string[]> {return this.$callMethod('matchingTableColumnsByCol',a1);}
  async maxHistory(...args: number[]): Promise<number> {return this.$callMethod('maxHistory',...args);}
  async minColumnWidth(...args: number[]): Promise<number> {return this.$callMethod('minColumnWidth',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseMoveB1(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMoveB1',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveAssetClass(a1: number,a2: number): Promise<string> {return this.$callMethod('moveAssetClass',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async navigateDown(): Promise<void> {return this.$callMethod('navigateDown');}
  async navigateLeft(): Promise<void> {return this.$callMethod('navigateLeft');}
  async navigateRight(): Promise<void> {return this.$callMethod('navigateRight');}
  async navigateUp(): Promise<void> {return this.$callMethod('navigateUp');}
  async paste(): Promise<void> {return this.$callMethod('paste');}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async pulldownHot(...args: number[]): Promise<number> {return this.$callMethod('pulldownHot',...args);}
  async pushHistory(): Promise<void> {return this.$callMethod('pushHistory');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async requestRedrawCanvas(): Promise<void> {return this.$callMethod('requestRedrawCanvas');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async rowHeight(...args: number[]): Promise<number> {return this.$callMethod('rowHeight',...args);}
  async rowYZoomed(a1: number): Promise<number> {return this.$callMethod('rowYZoomed',a1);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async scrollColStart(...args: number[]): Promise<number> {return this.$callMethod('scrollColStart',...args);}
  async scrollRowStart(...args: number[]): Promise<number> {return this.$callMethod('scrollRowStart',...args);}
  async selectIdx(...args: number[]): Promise<number> {return this.$callMethod('selectIdx',...args);}
  async selectedCellInTable(): Promise<boolean> {return this.$callMethod('selectedCellInTable');}
  async selectedCol(...args: number[]): Promise<number> {return this.$callMethod('selectedCol',...args);}
  async selectedRow(...args: number[]): Promise<number> {return this.$callMethod('selectedRow',...args);}
  async srcCol(...args: number[]): Promise<number> {return this.$callMethod('srcCol',...args);}
  async srcRow(...args: number[]): Promise<number> {return this.$callMethod('srcRow',...args);}
  async swapAssetClass(a1: number,a2: number): Promise<string> {return this.$callMethod('swapAssetClass',a1,a2);}
  async textIdx(a1: number): Promise<number> {return this.$callMethod('textIdx',a1);}
  async topTableOffset(...args: number[]): Promise<number> {return this.$callMethod('topTableOffset',...args);}
  async undo(a1: number): Promise<void> {return this.$callMethod('undo',a1);}
  async update(): Promise<void> {return this.$callMethod('update');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Group extends Item {
  bb: BoundingBox;
  bookmarks: Container<Bookmark>;
  createdIOvariables: Sequence<VariableBase>;
  groups: Sequence<Group>;
  inVariables: Sequence<VariableBase>;
  items: Sequence<Item>;
  outVariables: Sequence<VariableBase>;
  svgRenderer: SVGRenderer;
  wires: Sequence<Wire>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.bb=new BoundingBox(this.$prefix()+'.bb');
    this.bookmarks=new Container<Bookmark>(this.$prefix()+'.bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.$prefix()+'.createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.$prefix()+'.groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.$prefix()+'.inVariables',VariableBase);
    this.items=new Sequence<Item>(this.$prefix()+'.items',Item);
    this.outVariables=new Sequence<VariableBase>(this.$prefix()+'.outVariables',VariableBase);
    this.svgRenderer=new SVGRenderer(this.$prefix()+'.svgRenderer');
    this.wires=new Sequence<Wire>(this.$prefix()+'.wires',Wire);
  }
  async RESTProcess(a1: classdesc__RESTProcess_t,a2: string): Promise<void> {return this.$callMethod('RESTProcess',a1,a2);}
  async accessibleVars(): Promise<string[]> {return this.$callMethod('accessibleVars');}
  async addBookmark(a1: string): Promise<void> {return this.$callMethod('addBookmark',a1);}
  async addBookmarkXY(a1: number,a2: number,a3: string): Promise<void> {return this.$callMethod('addBookmarkXY',a1,a2,a3);}
  async addGroup(a1: Group): Promise<object> {return this.$callMethod('addGroup',a1);}
  async addInputVar(): Promise<void> {return this.$callMethod('addInputVar');}
  async addItem(a1: Item,a2: boolean): Promise<object> {return this.$callMethod('addItem',a1,a2);}
  async addOutputVar(): Promise<void> {return this.$callMethod('addOutputVar');}
  async addWire(...args: any[]): Promise<object> {return this.$callMethod('addWire',...args);}
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async adjustWiresGroup(a1: Wire): Promise<void> {return this.$callMethod('adjustWiresGroup',a1);}
  async arguments(): Promise<string> {return this.$callMethod('arguments');}
  async autoLayout(): Promise<void> {return this.$callMethod('autoLayout');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async bookmarkId(): Promise<string> {return this.$callMethod('bookmarkId');}
  async bookmarkList(): Promise<string[]> {return this.$callMethod('bookmarkList');}
  async bottom(): Promise<number> {return this.$callMethod('bottom');}
  async cBounds(): Promise<number[]> {return this.$callMethod('cBounds');}
  async checkAddIORegion(a1: Item): Promise<void> {return this.$callMethod('checkAddIORegion',a1);}
  async checkUnits(): Promise<object> {return this.$callMethod('checkUnits');}
  async classType(): Promise<string> {return this.$callMethod('classType');}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clone(): Promise<Item> {return this.$callMethod('clone');}
  async closestInPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestInPort',a1,a2);}
  async closestOutPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestOutPort',a1,a2);}
  async computeDisplayZoom(): Promise<number> {return this.$callMethod('computeDisplayZoom');}
  async computeRelZoom(): Promise<void> {return this.$callMethod('computeRelZoom');}
  async contains(...args: any[]): Promise<boolean> {return this.$callMethod('contains',...args);}
  async copy(): Promise<object> {return this.$callMethod('copy');}
  async copyUnowned(): Promise<object> {return this.$callMethod('copyUnowned');}
  async corners(): Promise<object[]> {return this.$callMethod('corners');}
  async defaultExtension(): Promise<string> {return this.$callMethod('defaultExtension');}
  async deleteAttachedWires(): Promise<void> {return this.$callMethod('deleteAttachedWires');}
  async deleteBookmark(a1: number): Promise<void> {return this.$callMethod('deleteBookmark',a1);}
  async deleteCallback(...args: string[]): Promise<string> {return this.$callMethod('deleteCallback',...args);}
  async deleteItem(a1: Item): Promise<void> {return this.$callMethod('deleteItem',a1);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async disableDelayedTooltip(): Promise<void> {return this.$callMethod('disableDelayedTooltip');}
  async displayContents(): Promise<boolean> {return this.$callMethod('displayContents');}
  async displayContentsChanged(): Promise<boolean> {return this.$callMethod('displayContentsChanged');}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async displayTooltip(a1: minsky__dummy,a2: string): Promise<void> {return this.$callMethod('displayTooltip',a1,a2);}
  async displayZoom(...args: number[]): Promise<number> {return this.$callMethod('displayZoom',...args);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async draw1edge(a1: Sequence<VariableBase>,a2: minsky__dummy,a3: number): Promise<void> {return this.$callMethod('draw1edge',a1,a2,a3);}
  async drawEdgeVariables(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawEdgeVariables',a1);}
  async drawIORegion(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawIORegion',a1);}
  async drawPorts(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawPorts',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async drawSelected(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawSelected',a1);}
  async dummyDraw(): Promise<void> {return this.$callMethod('dummyDraw');}
  async edgeScale(): Promise<number> {return this.$callMethod('edgeScale');}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async empty(): Promise<boolean> {return this.$callMethod('empty');}
  async ensureBBValid(): Promise<void> {return this.$callMethod('ensureBBValid');}
  async findGroup(a1: Group): Promise<object> {return this.$callMethod('findGroup',a1);}
  async findItem(a1: Item): Promise<object> {return this.$callMethod('findItem',a1);}
  async findWire(a1: Wire): Promise<object> {return this.$callMethod('findWire',a1);}
  async flip(): Promise<void> {return this.$callMethod('flip');}
  async flipContents(): Promise<void> {return this.$callMethod('flipContents');}
  async formula(): Promise<string> {return this.$callMethod('formula');}
  async globalGroup(): Promise<Group> {return this.$callMethod('globalGroup');}
  async gotoBookmark(a1: number): Promise<void> {return this.$callMethod('gotoBookmark',a1);}
  async gotoBookmark_b(a1: Bookmark): Promise<void> {return this.$callMethod('gotoBookmark_b',a1);}
  async height(): Promise<number> {return this.$callMethod('height');}
  async higher(a1: Group): Promise<boolean> {return this.$callMethod('higher',a1);}
  async iHeight(...args: any[]): Promise<number> {return this.$callMethod('iHeight',...args);}
  async iWidth(...args: any[]): Promise<number> {return this.$callMethod('iWidth',...args);}
  async id(): Promise<string> {return this.$callMethod('id');}
  async inIORegion(a1: number,a2: number): Promise<string> {return this.$callMethod('inIORegion',a1,a2);}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async ioVar(): Promise<boolean> {return this.$callMethod('ioVar');}
  async itemPtrFromThis(): Promise<object> {return this.$callMethod('itemPtrFromThis');}
  async json_pack(a1: classdesc__json_pack_t): Promise<void> {return this.$callMethod('json_pack',a1);}
  async left(): Promise<number> {return this.$callMethod('left');}
  async level(): Promise<number> {return this.$callMethod('level');}
  async localZoom(): Promise<number> {return this.$callMethod('localZoom');}
  async m_sf(...args: number[]): Promise<number> {return this.$callMethod('m_sf',...args);}
  async m_x(...args: number[]): Promise<number> {return this.$callMethod('m_x',...args);}
  async m_y(...args: number[]): Promise<number> {return this.$callMethod('m_y',...args);}
  async makeSubroutine(): Promise<void> {return this.$callMethod('makeSubroutine');}
  async margins(a1: number,a2: number): Promise<void> {return this.$callMethod('margins',a1,a2);}
  async marginsV(): Promise<number[]> {return this.$callMethod('marginsV');}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveContents(a1: Group): Promise<void> {return this.$callMethod('moveContents',a1);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async name(): Promise<string> {return this.$callMethod('name');}
  async nocycles(): Promise<boolean> {return this.$callMethod('nocycles');}
  async normaliseGroupRefs(a1: Group): Promise<void> {return this.$callMethod('normaliseGroupRefs',a1);}
  async numGroups(): Promise<number> {return this.$callMethod('numGroups');}
  async numItems(): Promise<number> {return this.$callMethod('numItems');}
  async numWires(): Promise<number> {return this.$callMethod('numWires');}
  async onBorder(...args: boolean[]): Promise<boolean> {return this.$callMethod('onBorder',...args);}
  async onItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onItem',a1,a2);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async onResizeHandles(...args: boolean[]): Promise<boolean> {return this.$callMethod('onResizeHandles',...args);}
  async portX(a1: number): Promise<number> {return this.$callMethod('portX',a1);}
  async portY(a1: number): Promise<number> {return this.$callMethod('portY',a1);}
  async ports(a1: number): Promise<object> {return this.$callMethod('ports',a1);}
  async portsSize(): Promise<number> {return this.$callMethod('portsSize');}
  async px(...args: number[]): Promise<number> {return this.$callMethod('px',...args);}
  async py(...args: number[]): Promise<number> {return this.$callMethod('py',...args);}
  async pz(...args: number[]): Promise<number> {return this.$callMethod('pz',...args);}
  async randomLayout(): Promise<void> {return this.$callMethod('randomLayout');}
  async relZoom(...args: number[]): Promise<number> {return this.$callMethod('relZoom',...args);}
  async removeDisplayPlot(): Promise<void> {return this.$callMethod('removeDisplayPlot');}
  async removeGroup(a1: Group): Promise<object> {return this.$callMethod('removeGroup',a1);}
  async removeItem(a1: Item): Promise<object> {return this.$callMethod('removeItem',a1);}
  async removeWire(a1: Wire): Promise<object> {return this.$callMethod('removeWire',a1);}
  async renameAllInstances(a1: string,a2: string): Promise<void> {return this.$callMethod('renameAllInstances',a1,a2);}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resizeHandleSize(): Promise<number> {return this.$callMethod('resizeHandleSize');}
  async resizeOnContents(): Promise<void> {return this.$callMethod('resizeOnContents');}
  async right(): Promise<number> {return this.$callMethod('right');}
  async rotFactor(): Promise<number> {return this.$callMethod('rotFactor');}
  async rotation(...args: any[]): Promise<number> {return this.$callMethod('rotation',...args);}
  async rotationAsRadians(): Promise<object> {return this.$callMethod('rotationAsRadians');}
  async scaleFactor(...args: any[]): Promise<number> {return this.$callMethod('scaleFactor',...args);}
  async select(a1: number,a2: number): Promise<object> {return this.$callMethod('select',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async setZoom(a1: number): Promise<void> {return this.$callMethod('setZoom',a1);}
  async splitBoundaryCrossingWires(): Promise<void> {return this.$callMethod('splitBoundaryCrossingWires');}
  async summariseGodleys(): Promise<object[]> {return this.$callMethod('summariseGodleys');}
  async throw_error(a1: string): Promise<void> {return this.$callMethod('throw_error',a1);}
  async title(...args: string[]): Promise<string> {return this.$callMethod('title',...args);}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async top(): Promise<number> {return this.$callMethod('top');}
  async uniqueItems(...args: any[]): Promise<boolean> {return this.$callMethod('uniqueItems',...args);}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async updateIcon(a1: number): Promise<void> {return this.$callMethod('updateIcon',a1);}
  async updateTimestamp(): Promise<void> {return this.$callMethod('updateTimestamp');}
  async value(): Promise<number> {return this.$callMethod('value');}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async x(): Promise<number> {return this.$callMethod('x');}
  async y(): Promise<number> {return this.$callMethod('y');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class GroupItems extends CppClass {
  bookmarks: Container<Bookmark>;
  createdIOvariables: Sequence<VariableBase>;
  groups: Sequence<Group>;
  inVariables: Sequence<VariableBase>;
  items: Sequence<Item>;
  outVariables: Sequence<VariableBase>;
  wires: Sequence<Wire>;
  constructor(prefix: string){
    super(prefix);
    this.bookmarks=new Container<Bookmark>(this.$prefix()+'.bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.$prefix()+'.createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.$prefix()+'.groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.$prefix()+'.inVariables',VariableBase);
    this.items=new Sequence<Item>(this.$prefix()+'.items',Item);
    this.outVariables=new Sequence<VariableBase>(this.$prefix()+'.outVariables',VariableBase);
    this.wires=new Sequence<Wire>(this.$prefix()+'.wires',Wire);
  }
  async addGroup(a1: Group): Promise<object> {return this.$callMethod('addGroup',a1);}
  async addItem(a1: Item,a2: boolean): Promise<object> {return this.$callMethod('addItem',a1,a2);}
  async addWire(...args: any[]): Promise<object> {return this.$callMethod('addWire',...args);}
  async adjustWiresGroup(a1: Wire): Promise<void> {return this.$callMethod('adjustWiresGroup',a1);}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async empty(): Promise<boolean> {return this.$callMethod('empty');}
  async findGroup(a1: Group): Promise<object> {return this.$callMethod('findGroup',a1);}
  async findItem(a1: Item): Promise<object> {return this.$callMethod('findItem',a1);}
  async findWire(a1: Wire): Promise<object> {return this.$callMethod('findWire',a1);}
  async nocycles(): Promise<boolean> {return this.$callMethod('nocycles');}
  async numGroups(): Promise<number> {return this.$callMethod('numGroups');}
  async numItems(): Promise<number> {return this.$callMethod('numItems');}
  async numWires(): Promise<number> {return this.$callMethod('numWires');}
  async removeDisplayPlot(): Promise<void> {return this.$callMethod('removeDisplayPlot');}
  async removeGroup(a1: Group): Promise<object> {return this.$callMethod('removeGroup',a1);}
  async removeItem(a1: Item): Promise<object> {return this.$callMethod('removeItem',a1);}
  async removeWire(a1: Wire): Promise<object> {return this.$callMethod('removeWire',a1);}
}

export class HandleLockInfo extends CppClass {
  handleNames: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.handleNames=new Sequence<string>(this.$prefix()+'.handleNames');
  }
  async calipers(...args: boolean[]): Promise<boolean> {return this.$callMethod('calipers',...args);}
  async order(...args: boolean[]): Promise<boolean> {return this.$callMethod('order',...args);}
  async orientation(...args: boolean[]): Promise<boolean> {return this.$callMethod('orientation',...args);}
  async slicer(...args: boolean[]): Promise<boolean> {return this.$callMethod('slicer',...args);}
}

export class IntOp extends Item {
  intVar: VariableBase;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.intVar=new VariableBase(this.$prefix()+'.intVar');
  }
  async coupled(): Promise<boolean> {return this.$callMethod('coupled');}
  async description(...args: string[]): Promise<string> {return this.$callMethod('description',...args);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async intVarOffset(...args: number[]): Promise<number> {return this.$callMethod('intVarOffset',...args);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async pack(a1: classdesc__pack_t,a2: string): Promise<void> {return this.$callMethod('pack',a1,a2);}
  async ports(a1: number): Promise<object> {return this.$callMethod('ports',a1);}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async toggleCoupled(): Promise<boolean> {return this.$callMethod('toggleCoupled');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async unpack(a1: classdesc__pack_t,a2: string): Promise<void> {return this.$callMethod('unpack',a1,a2);}
  async valueId(): Promise<string> {return this.$callMethod('valueId');}
}

export class LassoBox extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async angle(...args: number[]): Promise<number> {return this.$callMethod('angle',...args);}
  async contains(a1: Wire): Promise<boolean> {return this.$callMethod('contains',a1);}
  async x0(...args: number[]): Promise<number> {return this.$callMethod('x0',...args);}
  async x1(...args: number[]): Promise<number> {return this.$callMethod('x1',...args);}
  async y0(...args: number[]): Promise<number> {return this.$callMethod('y0',...args);}
  async y1(...args: number[]): Promise<number> {return this.$callMethod('y1',...args);}
}

export class Lock extends Item {
  lockedIcon: SVGRenderer;
  lockedState: ravel__RavelState;
  unlockedIcon: SVGRenderer;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.lockedIcon=new SVGRenderer(this.$prefix()+'.lockedIcon');
    this.lockedState=new ravel__RavelState(this.$prefix()+'.lockedState');
    this.unlockedIcon=new SVGRenderer(this.$prefix()+'.unlockedIcon');
  }
  async addPorts(): Promise<void> {return this.$callMethod('addPorts');}
  async applyLockedStateToRavel(): Promise<void> {return this.$callMethod('applyLockedStateToRavel');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async locked(): Promise<boolean> {return this.$callMethod('locked');}
  async ravelInput(): Promise<Ravel> {return this.$callMethod('ravelInput');}
  async toggleLocked(): Promise<void> {return this.$callMethod('toggleLocked');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
}

export class Minsky extends CppClass {
  canvas: Canvas;
  conversions: civita__Conversions;
  dimensions: Map<string,civita__Dimension>;
  equationDisplay: EquationDisplay;
  evalGodley: EvalGodley;
  flowVars: Sequence<number>;
  fontSampler: FontDisplay;
  histogramResource: SVGRenderer;
  logVarList: Container<string>;
  maxFlowValue: Map<Units,number>;
  maxValue: Map<Units,number>;
  model: Group;
  namedItems: Map<string,Item>;
  phillipsDiagram: PhillipsDiagram;
  publicationTabs: Sequence<PubTab>;
  stockVars: Sequence<number>;
  variableInstanceList: VariableInstanceList;
  variablePane: VariablePane;
  variableValues: VariableValues;
  constructor(prefix: string){
    super(prefix);
    this.canvas=new Canvas(this.$prefix()+'.canvas');
    this.conversions=new civita__Conversions(this.$prefix()+'.conversions');
    this.dimensions=new Map<string,civita__Dimension>(this.$prefix()+'.dimensions',civita__Dimension);
    this.equationDisplay=new EquationDisplay(this.$prefix()+'.equationDisplay');
    this.evalGodley=new EvalGodley(this.$prefix()+'.evalGodley');
    this.flowVars=new Sequence<number>(this.$prefix()+'.flowVars');
    this.fontSampler=new FontDisplay(this.$prefix()+'.fontSampler');
    this.histogramResource=new SVGRenderer(this.$prefix()+'.histogramResource');
    this.logVarList=new Container<string>(this.$prefix()+'.logVarList');
    this.maxFlowValue=new Map<Units,number>(this.$prefix()+'.maxFlowValue');
    this.maxValue=new Map<Units,number>(this.$prefix()+'.maxValue');
    this.model=new Group(this.$prefix()+'.model');
    this.namedItems=new Map<string,Item>(this.$prefix()+'.namedItems',Item);
    this.phillipsDiagram=new PhillipsDiagram(this.$prefix()+'.phillipsDiagram');
    this.publicationTabs=new Sequence<PubTab>(this.$prefix()+'.publicationTabs',PubTab);
    this.stockVars=new Sequence<number>(this.$prefix()+'.stockVars');
    this.variableInstanceList=new VariableInstanceList(this.$prefix()+'.variableInstanceList');
    this.variablePane=new VariablePane(this.$prefix()+'.variablePane');
    this.variableValues=new VariableValues(this.$prefix()+'.variableValues');
  }
  async addCanvasItemToPublicationTab(a1: number): Promise<void> {return this.$callMethod('addCanvasItemToPublicationTab',a1);}
  async addIntegral(): Promise<void> {return this.$callMethod('addIntegral');}
  async addNewPublicationTab(a1: string): Promise<void> {return this.$callMethod('addNewPublicationTab',a1);}
  async allGodleyFlowVars(): Promise<string[]> {return this.$callMethod('allGodleyFlowVars');}
  async assetClasses(): Promise<string[]> {return this.$callMethod('assetClasses');}
  async autoLayout(): Promise<void> {return this.$callMethod('autoLayout');}
  async autoSaveFile(): Promise<string> {return this.$callMethod('autoSaveFile');}
  async availableOperations(): Promise<string[]> {return this.$callMethod('availableOperations');}
  async availableOperationsMapping(): Promise<object[]> {return this.$callMethod('availableOperationsMapping');}
  async balanceDuplicateColumns(a1: GodleyIcon,a2: number): Promise<void> {return this.$callMethod('balanceDuplicateColumns',a1,a2);}
  async bookmarkRefresh(): Promise<void> {return this.$callMethod('bookmarkRefresh');}
  async checkEquationOrder(): Promise<boolean> {return this.$callMethod('checkEquationOrder');}
  async checkMemAllocation(a1: number): Promise<string> {return this.$callMethod('checkMemAllocation',a1);}
  async checkPushHistory(): Promise<void> {return this.$callMethod('checkPushHistory');}
  async classifyOp(a1: string): Promise<string> {return this.$callMethod('classifyOp',a1);}
  async clearAllMaps(...args: boolean[]): Promise<void> {return this.$callMethod('clearAllMaps',...args);}
  async clearAllMapsTCL(): Promise<void> {return this.$callMethod('clearAllMapsTCL');}
  async clearBusyCursor(): Promise<void> {return this.$callMethod('clearBusyCursor');}
  async clearHistory(): Promise<void> {return this.$callMethod('clearHistory');}
  async clipboardEmpty(): Promise<boolean> {return this.$callMethod('clipboardEmpty');}
  async closeLogFile(): Promise<void> {return this.$callMethod('closeLogFile');}
  async commandHook(a1: string,a2: number): Promise<boolean> {return this.$callMethod('commandHook',a1,a2);}
  async constructEquations(): Promise<void> {return this.$callMethod('constructEquations');}
  async convertVarType(a1: string,a2: string): Promise<void> {return this.$callMethod('convertVarType',a1,a2);}
  async copy(): Promise<void> {return this.$callMethod('copy');}
  async cut(): Promise<void> {return this.$callMethod('cut');}
  async cycleCheck(): Promise<boolean> {return this.$callMethod('cycleCheck');}
  async daysUntilRavelExpires(): Promise<number> {return this.$callMethod('daysUntilRavelExpires');}
  async defaultFont(...args: any[]): Promise<string> {return this.$callMethod('defaultFont',...args);}
  async definingVar(a1: string): Promise<object> {return this.$callMethod('definingVar',a1);}
  async deleteAllUnits(): Promise<void> {return this.$callMethod('deleteAllUnits');}
  async deltaT(): Promise<number> {return this.$callMethod('deltaT');}
  async dimensionalAnalysis(): Promise<void> {return this.$callMethod('dimensionalAnalysis');}
  async displayErrorItem(a1: Item): Promise<void> {return this.$callMethod('displayErrorItem',a1);}
  async displayStyle(...args: string[]): Promise<string> {return this.$callMethod('displayStyle',...args);}
  async displayValues(...args: boolean[]): Promise<boolean> {return this.$callMethod('displayValues',...args);}
  async doPushHistory(...args: boolean[]): Promise<boolean> {return this.$callMethod('doPushHistory',...args);}
  async ecolabVersion(): Promise<string> {return this.$callMethod('ecolabVersion');}
  async edited(): Promise<boolean> {return this.$callMethod('edited');}
  async epsAbs(...args: number[]): Promise<number> {return this.$callMethod('epsAbs',...args);}
  async epsRel(...args: number[]): Promise<number> {return this.$callMethod('epsRel',...args);}
  async evalEquations(): Promise<void> {return this.$callMethod('evalEquations');}
  async exportAllPlotsAsCSV(a1: string): Promise<void> {return this.$callMethod('exportAllPlotsAsCSV',a1);}
  async exportSchema(a1: string,a2: number): Promise<void> {return this.$callMethod('exportSchema',a1,a2);}
  async fileVersion(...args: string[]): Promise<string> {return this.$callMethod('fileVersion',...args);}
  async fontScale(...args: any[]): Promise<number> {return this.$callMethod('fontScale',...args);}
  async garbageCollect(): Promise<void> {return this.$callMethod('garbageCollect');}
  async getCommandData(a1: string): Promise<string> {return this.$callMethod('getCommandData',a1);}
  async getc(): Promise<number> {return this.$callMethod('getc');}
  async implicit(...args: boolean[]): Promise<boolean> {return this.$callMethod('implicit',...args);}
  async importDuplicateColumn(a1: GodleyTable,a2: number): Promise<void> {return this.$callMethod('importDuplicateColumn',a1,a2);}
  async importVensim(a1: string): Promise<void> {return this.$callMethod('importVensim',a1);}
  async imposeDimensions(): Promise<void> {return this.$callMethod('imposeDimensions');}
  async initGodleys(): Promise<void> {return this.$callMethod('initGodleys');}
  async inputWired(a1: string): Promise<boolean> {return this.$callMethod('inputWired',a1);}
  async insertGroupFromFile(a1: string): Promise<void> {return this.$callMethod('insertGroupFromFile',a1);}
  async itemFromNamedItem(a1: string): Promise<void> {return this.$callMethod('itemFromNamedItem',a1);}
  async lastT(...args: number[]): Promise<number> {return this.$callMethod('lastT',...args);}
  async latex(a1: string,a2: boolean): Promise<void> {return this.$callMethod('latex',a1,a2);}
  async latex2pango(a1: string): Promise<string> {return this.$callMethod('latex2pango',a1);}
  async listAllInstances(): Promise<void> {return this.$callMethod('listAllInstances');}
  async listFonts(): Promise<string[]> {return this.$callMethod('listFonts');}
  async load(a1: string): Promise<void> {return this.$callMethod('load',a1);}
  async loggingEnabled(): Promise<boolean> {return this.$callMethod('loggingEnabled');}
  async makeVariablesConsistent(): Promise<void> {return this.$callMethod('makeVariablesConsistent');}
  async markEdited(): Promise<void> {return this.$callMethod('markEdited');}
  async matchingTableColumns(a1: GodleyIcon,a2: string): Promise<string[]> {return this.$callMethod('matchingTableColumns',a1,a2);}
  async matlab(a1: string): Promise<void> {return this.$callMethod('matlab',a1);}
  async maxHistory(...args: number[]): Promise<number> {return this.$callMethod('maxHistory',...args);}
  async maxWaitMS(...args: number[]): Promise<number> {return this.$callMethod('maxWaitMS',...args);}
  async message(a1: string): Promise<void> {return this.$callMethod('message',a1);}
  async minskyVersion(...args: string[]): Promise<string> {return this.$callMethod('minskyVersion',...args);}
  async multipleEquities(...args: any[]): Promise<boolean> {return this.$callMethod('multipleEquities',...args);}
  async nSteps(...args: number[]): Promise<number> {return this.$callMethod('nSteps',...args);}
  async nameCurrentItem(a1: string): Promise<void> {return this.$callMethod('nameCurrentItem',a1);}
  async numBackups(...args: number[]): Promise<number> {return this.$callMethod('numBackups',...args);}
  async numOpArgs(a1: string): Promise<number> {return this.$callMethod('numOpArgs',a1);}
  async openGroupInCanvas(): Promise<void> {return this.$callMethod('openGroupInCanvas');}
  async openLogFile(a1: string): Promise<void> {return this.$callMethod('openLogFile',a1);}
  async openModelInCanvas(): Promise<void> {return this.$callMethod('openModelInCanvas');}
  async order(...args: number[]): Promise<number> {return this.$callMethod('order',...args);}
  async paste(): Promise<void> {return this.$callMethod('paste');}
  async physicalMem(): Promise<number> {return this.$callMethod('physicalMem');}
  async popFlags(): Promise<void> {return this.$callMethod('popFlags');}
  async populateMissingDimensions(): Promise<void> {return this.$callMethod('populateMissingDimensions');}
  async populateMissingDimensionsFromVariable(...args: any[]): Promise<void> {return this.$callMethod('populateMissingDimensionsFromVariable',...args);}
  async progress(a1: string,a2: number): Promise<void> {return this.$callMethod('progress',a1,a2);}
  async pushFlags(): Promise<void> {return this.$callMethod('pushFlags');}
  async pushHistory(): Promise<boolean> {return this.$callMethod('pushHistory');}
  async randomLayout(): Promise<void> {return this.$callMethod('randomLayout');}
  async ravelAvailable(): Promise<boolean> {return this.$callMethod('ravelAvailable');}
  async ravelExpired(): Promise<boolean> {return this.$callMethod('ravelExpired');}
  async ravelVersion(): Promise<string> {return this.$callMethod('ravelVersion');}
  async redrawAllGodleyTables(): Promise<void> {return this.$callMethod('redrawAllGodleyTables');}
  async reloadAllCSVParameters(): Promise<void> {return this.$callMethod('reloadAllCSVParameters');}
  async renameDimension(a1: string,a2: string): Promise<void> {return this.$callMethod('renameDimension',a1,a2);}
  async renderAllPlotsAsSVG(a1: string): Promise<void> {return this.$callMethod('renderAllPlotsAsSVG',a1);}
  async renderCanvasToEMF(a1: string): Promise<void> {return this.$callMethod('renderCanvasToEMF',a1);}
  async renderCanvasToPDF(a1: string): Promise<void> {return this.$callMethod('renderCanvasToPDF',a1);}
  async renderCanvasToPNG(...args: any[]): Promise<void> {return this.$callMethod('renderCanvasToPNG',...args);}
  async renderCanvasToPS(a1: string): Promise<void> {return this.$callMethod('renderCanvasToPS',a1);}
  async renderCanvasToSVG(a1: string): Promise<void> {return this.$callMethod('renderCanvasToSVG',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async requestReset(): Promise<void> {return this.$callMethod('requestReset');}
  async reset(): Promise<void> {return this.$callMethod('reset');}
  async resetIfFlagged(): Promise<boolean> {return this.$callMethod('resetIfFlagged');}
  async resetScroll(): Promise<void> {return this.$callMethod('resetScroll');}
  async reset_flag(): Promise<boolean> {return this.$callMethod('reset_flag');}
  async reverse(...args: boolean[]): Promise<boolean> {return this.$callMethod('reverse',...args);}
  async rkreset(): Promise<void> {return this.$callMethod('rkreset');}
  async rkstep(): Promise<void> {return this.$callMethod('rkstep');}
  async runItemDeletedCallback(a1: Item): Promise<void> {return this.$callMethod('runItemDeletedCallback',a1);}
  async running(...args: boolean[]): Promise<boolean> {return this.$callMethod('running',...args);}
  async save(a1: string): Promise<void> {return this.$callMethod('save',a1);}
  async saveCanvasItemAsFile(a1: string): Promise<void> {return this.$callMethod('saveCanvasItemAsFile',a1);}
  async saveGroupAsFile(a1: Group,a2: string): Promise<void> {return this.$callMethod('saveGroupAsFile',a1,a2);}
  async saveSelectionAsFile(a1: string): Promise<void> {return this.$callMethod('saveSelectionAsFile',a1);}
  async setAllDEmode(a1: boolean): Promise<void> {return this.$callMethod('setAllDEmode',a1);}
  async setAutoSaveFile(a1: string): Promise<void> {return this.$callMethod('setAutoSaveFile',a1);}
  async setBusyCursor(): Promise<void> {return this.$callMethod('setBusyCursor');}
  async setDefinition(a1: string,a2: string): Promise<void> {return this.$callMethod('setDefinition',a1,a2);}
  async setGodleyDisplayValue(a1: boolean,a2: string): Promise<void> {return this.$callMethod('setGodleyDisplayValue',a1,a2);}
  async setGodleyIconResource(a1: string): Promise<void> {return this.$callMethod('setGodleyIconResource',a1);}
  async setGroupIconResource(a1: string): Promise<void> {return this.$callMethod('setGroupIconResource',a1);}
  async setLockIconResource(a1: string,a2: string): Promise<void> {return this.$callMethod('setLockIconResource',a1,a2);}
  async setRavelIconResource(a1: string): Promise<void> {return this.$callMethod('setRavelIconResource',a1);}
  async simulationDelay(...args: number[]): Promise<number> {return this.$callMethod('simulationDelay',...args);}
  async srand(a1: number): Promise<void> {return this.$callMethod('srand',a1);}
  async step(): Promise<number[]> {return this.$callMethod('step');}
  async stepMax(...args: number[]): Promise<number> {return this.$callMethod('stepMax',...args);}
  async stepMin(...args: number[]): Promise<number> {return this.$callMethod('stepMin',...args);}
  async t(...args: number[]): Promise<number> {return this.$callMethod('t',...args);}
  async t0(...args: number[]): Promise<number> {return this.$callMethod('t0',...args);}
  async timeUnit(...args: string[]): Promise<string> {return this.$callMethod('timeUnit',...args);}
  async tmax(...args: number[]): Promise<number> {return this.$callMethod('tmax',...args);}
  async triggerCheckMemAllocationCallback(): Promise<boolean> {return this.$callMethod('triggerCheckMemAllocationCallback');}
  async undo(a1: number): Promise<number> {return this.$callMethod('undo',a1);}
  async variableTypes(): Promise<string[]> {return this.$callMethod('variableTypes');}
}

export class PhillipsDiagram extends RenderNativeWindow {
  backgroundColour: ecolab__cairo__Colour;
  flows: Map<Pair<string,string>,PhillipsFlow>;
  item: Item;
  stocks: Map<string,PhillipsStock>;
  wire: Wire;
  constructor(prefix: string|RenderNativeWindow){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.flows=new Map<Pair<string,string>,PhillipsFlow>(this.$prefix()+'.flows',PhillipsFlow);
    this.item=new Item(this.$prefix()+'.item');
    this.stocks=new Map<string,PhillipsStock>(this.$prefix()+'.stocks',PhillipsStock);
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async startRotatingItem(a1: number,a2: number): Promise<void> {return this.$callMethod('startRotatingItem',a1,a2);}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class PhillipsFlow extends Item {
  maxFlow: Map<Units,number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.maxFlow=new Map<Units,number>(this.$prefix()+'.maxFlow');
  }
  async addTerm(a1: number,a2: string): Promise<void> {return this.$callMethod('addTerm',a1,a2);}
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async coords(...args: any[]): Promise<number[]> {return this.$callMethod('coords',...args);}
  async deleteHandle(a1: number,a2: number): Promise<void> {return this.$callMethod('deleteHandle',a1,a2);}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async draw(...args: any[]): Promise<void> {return this.$callMethod('draw',...args);}
  async editHandle(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('editHandle',a1,a2,a3);}
  async from(): Promise<object> {return this.$callMethod('from');}
  async insertHandle(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('insertHandle',a1,a2,a3);}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveIntoGroup(a1: Group): Promise<void> {return this.$callMethod('moveIntoGroup',a1);}
  async moveToPorts(a1: Port,a2: Port): Promise<void> {return this.$callMethod('moveToPorts',a1,a2);}
  async near(a1: number,a2: number): Promise<boolean> {return this.$callMethod('near',a1,a2);}
  async nearestHandle(a1: number,a2: number): Promise<number> {return this.$callMethod('nearestHandle',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async split(): Promise<void> {return this.$callMethod('split');}
  async storeCairoCoords(a1: minsky__dummy): Promise<void> {return this.$callMethod('storeCairoCoords',a1);}
  async straighten(): Promise<void> {return this.$callMethod('straighten');}
  async to(): Promise<object> {return this.$callMethod('to');}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async units(...args: boolean[]): Promise<object> {return this.$callMethod('units',...args);}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async value(): Promise<number> {return this.$callMethod('value');}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
}

export class PhillipsStock extends Item {
  maxStock: Map<Units,number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.maxStock=new Map<Units,number>(this.$prefix()+'.maxStock');
  }
  async classType(): Promise<string> {return this.$callMethod('classType');}
  async clone(): Promise<minsky__Variable<minsky__VariableType__TypeT>> {return this.$callMethod('clone');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async numPorts(): Promise<number> {return this.$callMethod('numPorts');}
  async type(): Promise<string> {return this.$callMethod('type');}
}

export class PlotWidget extends Item {
  backgroundColour: ecolab__cairo__Colour;
  horizontalMarkers: Sequence<string>;
  item: Item;
  palette: Sequence<ecolab__Plot__LineStyle>;
  penLabels: Sequence<string>;
  verticalMarkers: Sequence<string>;
  wire: Wire;
  xmaxVar: VariableValue;
  xminVar: VariableValue;
  xvars: Sequence<VariableValue>;
  y1maxVar: VariableValue;
  y1minVar: VariableValue;
  ymaxVar: VariableValue;
  yminVar: VariableValue;
  yvars: Sequence<Sequence<VariableValue>>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.horizontalMarkers=new Sequence<string>(this.$prefix()+'.horizontalMarkers');
    this.item=new Item(this.$prefix()+'.item');
    this.palette=new Sequence<ecolab__Plot__LineStyle>(this.$prefix()+'.palette',ecolab__Plot__LineStyle);
    this.penLabels=new Sequence<string>(this.$prefix()+'.penLabels');
    this.verticalMarkers=new Sequence<string>(this.$prefix()+'.verticalMarkers');
    this.wire=new Wire(this.$prefix()+'.wire');
    this.xmaxVar=new VariableValue(this.$prefix()+'.xmaxVar');
    this.xminVar=new VariableValue(this.$prefix()+'.xminVar');
    this.xvars=new Sequence<VariableValue>(this.$prefix()+'.xvars',VariableValue);
    this.y1maxVar=new VariableValue(this.$prefix()+'.y1maxVar');
    this.y1minVar=new VariableValue(this.$prefix()+'.y1minVar');
    this.ymaxVar=new VariableValue(this.$prefix()+'.ymaxVar');
    this.yminVar=new VariableValue(this.$prefix()+'.yminVar');
    this.yvars=new Sequence<Sequence<VariableValue>>(this.$prefix()+'.yvars',Sequence<VariableValue>);
  }
  async AssignSide(a1: number,a2: string): Promise<void> {return this.$callMethod('AssignSide',a1,a2);}
  async Image(...args: any[]): Promise<string> {return this.$callMethod('Image',...args);}
  async LabelPen(a1: number,a2: ecolab__cairo__Surface): Promise<void> {return this.$callMethod('LabelPen',a1,a2);}
  async addConstantCurves(): Promise<void> {return this.$callMethod('addConstantCurves');}
  async addPlotPt(a1: number): Promise<void> {return this.$callMethod('addPlotPt',a1);}
  async addPorts(): Promise<void> {return this.$callMethod('addPorts');}
  async addPt(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('addPt',a1,a2,a3);}
  async assignSide(a1: number,a2: string): Promise<void> {return this.$callMethod('assignSide',a1,a2);}
  async autoScale(): Promise<void> {return this.$callMethod('autoScale');}
  async autoscale(...args: boolean[]): Promise<boolean> {return this.$callMethod('autoscale',...args);}
  async availableMarkers(): Promise<string[]> {return this.$callMethod('availableMarkers');}
  async barWidth(...args: any[]): Promise<number> {return this.$callMethod('barWidth',...args);}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async clearPenAttributes(): Promise<void> {return this.$callMethod('clearPenAttributes');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async connectVar(a1: VariableValue,a2: number): Promise<void> {return this.$callMethod('connectVar',a1,a2);}
  async contains(a1: number,a2: number): Promise<boolean> {return this.$callMethod('contains',a1,a2);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async disconnectAllVars(): Promise<void> {return this.$callMethod('disconnectAllVars');}
  async displayFontSize(...args: number[]): Promise<number> {return this.$callMethod('displayFontSize',...args);}
  async displayNTicks(...args: number[]): Promise<number> {return this.$callMethod('displayNTicks',...args);}
  async draw(...args: any[]): Promise<void> {return this.$callMethod('draw',...args);}
  async exp_threshold(...args: number[]): Promise<number> {return this.$callMethod('exp_threshold',...args);}
  async exportAsCSV(...args: any[]): Promise<void> {return this.$callMethod('exportAsCSV',...args);}
  async extendPalette(): Promise<void> {return this.$callMethod('extendPalette');}
  async fontScale(...args: number[]): Promise<number> {return this.$callMethod('fontScale',...args);}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async grid(...args: boolean[]): Promise<boolean> {return this.$callMethod('grid',...args);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async labelPen(a1: number,a2: string): Promise<void> {return this.$callMethod('labelPen',a1,a2);}
  async leadingMarker(...args: boolean[]): Promise<boolean> {return this.$callMethod('leadingMarker',...args);}
  async legend(...args: boolean[]): Promise<boolean> {return this.$callMethod('legend',...args);}
  async legendFontSz(...args: number[]): Promise<number> {return this.$callMethod('legendFontSz',...args);}
  async legendLeft(...args: number[]): Promise<number> {return this.$callMethod('legendLeft',...args);}
  async legendOffset(...args: number[]): Promise<number> {return this.$callMethod('legendOffset',...args);}
  async legendSide(...args: string[]): Promise<string> {return this.$callMethod('legendSide',...args);}
  async legendTop(...args: number[]): Promise<number> {return this.$callMethod('legendTop',...args);}
  async lh(a1: number,a2: number): Promise<number> {return this.$callMethod('lh',a1,a2);}
  async logx(...args: boolean[]): Promise<boolean> {return this.$callMethod('logx',...args);}
  async logy(...args: boolean[]): Promise<boolean> {return this.$callMethod('logy',...args);}
  async makeDisplayPlot(): Promise<void> {return this.$callMethod('makeDisplayPlot');}
  async maxx(...args: number[]): Promise<number> {return this.$callMethod('maxx',...args);}
  async maxy(...args: number[]): Promise<number> {return this.$callMethod('maxy',...args);}
  async maxy1(...args: number[]): Promise<number> {return this.$callMethod('maxy1',...args);}
  async minx(...args: number[]): Promise<number> {return this.$callMethod('minx',...args);}
  async miny(...args: number[]): Promise<number> {return this.$callMethod('miny',...args);}
  async miny1(...args: number[]): Promise<number> {return this.$callMethod('miny1',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async nBoundsPorts(...args: number[]): Promise<number> {return this.$callMethod('nBoundsPorts',...args);}
  async numLines(...args: any[]): Promise<number> {return this.$callMethod('numLines',...args);}
  async nxTicks(...args: number[]): Promise<number> {return this.$callMethod('nxTicks',...args);}
  async nyTicks(...args: number[]): Promise<number> {return this.$callMethod('nyTicks',...args);}
  async offx(...args: number[]): Promise<number> {return this.$callMethod('offx',...args);}
  async offy(...args: number[]): Promise<number> {return this.$callMethod('offy',...args);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async percent(...args: boolean[]): Promise<boolean> {return this.$callMethod('percent',...args);}
  async plotType(...args: string[]): Promise<string> {return this.$callMethod('plotType',...args);}
  async plotWidgetCast(): Promise<PlotWidget> {return this.$callMethod('plotWidgetCast');}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async redraw(): Promise<boolean> {return this.$callMethod('redraw');}
  async redrawWithBounds(): Promise<void> {return this.$callMethod('redrawWithBounds');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async scalePlot(): Promise<void> {return this.$callMethod('scalePlot');}
  async setMinMax(): Promise<void> {return this.$callMethod('setMinMax');}
  async subgrid(...args: boolean[]): Promise<boolean> {return this.$callMethod('subgrid',...args);}
  async symbolEvery(...args: number[]): Promise<number> {return this.$callMethod('symbolEvery',...args);}
  async title(...args: string[]): Promise<string> {return this.$callMethod('title',...args);}
  async updateIcon(a1: number): Promise<void> {return this.$callMethod('updateIcon',a1);}
  async width(): Promise<number> {return this.$callMethod('width');}
  async xlabel(...args: any[]): Promise<string> {return this.$callMethod('xlabel',...args);}
  async xmax(...args: number[]): Promise<number> {return this.$callMethod('xmax',...args);}
  async xmin(...args: number[]): Promise<number> {return this.$callMethod('xmin',...args);}
  async xtickAngle(...args: number[]): Promise<number> {return this.$callMethod('xtickAngle',...args);}
  async y1label(...args: any[]): Promise<string> {return this.$callMethod('y1label',...args);}
  async y1max(...args: number[]): Promise<number> {return this.$callMethod('y1max',...args);}
  async y1min(...args: number[]): Promise<number> {return this.$callMethod('y1min',...args);}
  async ylabel(...args: any[]): Promise<string> {return this.$callMethod('ylabel',...args);}
  async ymax(...args: number[]): Promise<number> {return this.$callMethod('ymax',...args);}
  async ymin(...args: number[]): Promise<number> {return this.$callMethod('ymin',...args);}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Port extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async checkUnits(): Promise<object> {return this.$callMethod('checkUnits');}
  async combineInput(a1: number,a2: number): Promise<void> {return this.$callMethod('combineInput',a1,a2);}
  async deleteWires(): Promise<void> {return this.$callMethod('deleteWires');}
  async eraseWire(a1: Wire): Promise<void> {return this.$callMethod('eraseWire',a1);}
  async identity(): Promise<number> {return this.$callMethod('identity');}
  async input(): Promise<boolean> {return this.$callMethod('input');}
  async item(): Promise<Item> {return this.$callMethod('item');}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async multiWireAllowed(): Promise<boolean> {return this.$callMethod('multiWireAllowed');}
  async numWires(): Promise<number> {return this.$callMethod('numWires');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async value(): Promise<number> {return this.$callMethod('value');}
  async wires(): Promise<Sequence<Wire>> {return this.$callMethod('wires');}
  async x(): Promise<number> {return this.$callMethod('x');}
  async y(): Promise<number> {return this.$callMethod('y');}
}

export class PubItem extends CppClass {
  itemRef: Item;
  constructor(prefix: string){
    super(prefix);
    this.itemRef=new Item(this.$prefix()+'.itemRef');
  }
  async editorMode(...args: boolean[]): Promise<boolean> {return this.$callMethod('editorMode',...args);}
  async item(...args: number[]): Promise<number> {return this.$callMethod('item',...args);}
  async itemCoords(a1: number,a2: number): Promise<object> {return this.$callMethod('itemCoords',a1,a2);}
  async rotation(...args: number[]): Promise<number> {return this.$callMethod('rotation',...args);}
  async x(...args: number[]): Promise<number> {return this.$callMethod('x',...args);}
  async y(...args: number[]): Promise<number> {return this.$callMethod('y',...args);}
  async zoomX(...args: number[]): Promise<number> {return this.$callMethod('zoomX',...args);}
  async zoomY(...args: number[]): Promise<number> {return this.$callMethod('zoomY',...args);}
}

export class PubTab extends RenderNativeWindow {
  items: Sequence<PubItem>;
  constructor(prefix: string|RenderNativeWindow){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.items=new Sequence<PubItem>(this.$prefix()+'.items',PubItem);
  }
  async addNote(a1: string,a2: number,a3: number): Promise<void> {return this.$callMethod('addNote',a1,a2,a3);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async m_zoomFactor(...args: number[]): Promise<number> {return this.$callMethod('m_zoomFactor',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async name(...args: string[]): Promise<string> {return this.$callMethod('name',...args);}
  async offsx(...args: number[]): Promise<number> {return this.$callMethod('offsx',...args);}
  async offsy(...args: number[]): Promise<number> {return this.$callMethod('offsy',...args);}
  async panning(...args: boolean[]): Promise<boolean> {return this.$callMethod('panning',...args);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async removeItemAt(a1: number,a2: number): Promise<void> {return this.$callMethod('removeItemAt',a1,a2);}
  async removeSelf(): Promise<void> {return this.$callMethod('removeSelf');}
  async rotateItemAt(a1: number,a2: number): Promise<void> {return this.$callMethod('rotateItemAt',a1,a2);}
  async toggleEditorModeAt(a1: number,a2: number): Promise<void> {return this.$callMethod('toggleEditorModeAt',a1,a2);}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Ravel extends Item {
  axisDimensions: Map<string,civita__Dimension>;
  db: ravelCAPI__Database;
  lockGroup: RavelLockGroup;
  popup: RavelPopup;
  svgRenderer: SVGRenderer;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.axisDimensions=new Map<string,civita__Dimension>(this.$prefix()+'.axisDimensions',civita__Dimension);
    this.db=new ravelCAPI__Database(this.$prefix()+'.db');
    this.lockGroup=new RavelLockGroup(this.$prefix()+'.lockGroup');
    this.popup=new RavelPopup(this.$prefix()+'.popup');
    this.svgRenderer=new SVGRenderer(this.$prefix()+'.svgRenderer');
  }
  async adjustSlicer(a1: number): Promise<void> {return this.$callMethod('adjustSlicer',a1);}
  async allSliceLabels(): Promise<string[]> {return this.$callMethod('allSliceLabels');}
  async allSliceLabelsAxis(a1: number): Promise<string[]> {return this.$callMethod('allSliceLabelsAxis',a1);}
  async applyState(a1: ravel__RavelState): Promise<void> {return this.$callMethod('applyState',a1);}
  async broadcastStateToLockGroup(): Promise<void> {return this.$callMethod('broadcastStateToLockGroup');}
  async collapseAllHandles(a1: boolean): Promise<void> {return this.$callMethod('collapseAllHandles',a1);}
  async description(): Promise<string> {return this.$callMethod('description');}
  async dimension(a1: number): Promise<object> {return this.$callMethod('dimension',a1);}
  async dimensionType(...args: any[]): Promise<string> {return this.$callMethod('dimensionType',...args);}
  async dimensionUnitsFormat(...args: any[]): Promise<string> {return this.$callMethod('dimensionUnitsFormat',...args);}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async displayFilterCaliper(): Promise<boolean> {return this.$callMethod('displayFilterCaliper');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async exportAsCSV(a1: string,a2: boolean): Promise<void> {return this.$callMethod('exportAsCSV',a1,a2);}
  async flipped(...args: boolean[]): Promise<boolean> {return this.$callMethod('flipped',...args);}
  async getState(): Promise<object> {return this.$callMethod('getState');}
  async handleDescription(a1: number): Promise<string> {return this.$callMethod('handleDescription',a1);}
  async handleNames(): Promise<string[]> {return this.$callMethod('handleNames');}
  async handleSetReduction(a1: number,a2: string): Promise<void> {return this.$callMethod('handleSetReduction',a1,a2);}
  async handleSortableByValue(): Promise<boolean> {return this.$callMethod('handleSortableByValue');}
  async hypercube(): Promise<object> {return this.$callMethod('hypercube');}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async initRavelFromDb(): Promise<void> {return this.$callMethod('initRavelFromDb');}
  async joinLockGroup(a1: number): Promise<void> {return this.$callMethod('joinLockGroup',a1);}
  async leaveLockGroup(): Promise<void> {return this.$callMethod('leaveLockGroup');}
  async lockGroupColours(): Promise<number[]> {return this.$callMethod('lockGroupColours');}
  async maxRank(): Promise<number> {return this.$callMethod('maxRank');}
  async nextReduction(a1: string): Promise<void> {return this.$callMethod('nextReduction',a1);}
  async numHandles(): Promise<number> {return this.$callMethod('numHandles');}
  async numSliceLabels(a1: number): Promise<number> {return this.$callMethod('numSliceLabels',a1);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async pickSliceLabels(a1: number,a2: string[]): Promise<void> {return this.$callMethod('pickSliceLabels',a1,a2);}
  async pickedSliceLabels(...args: any[]): Promise<string[]> {return this.$callMethod('pickedSliceLabels',...args);}
  async populateHypercube(a1: civita__Hypercube): Promise<void> {return this.$callMethod('populateHypercube',a1);}
  async radius(): Promise<number> {return this.$callMethod('radius');}
  async ravelCast(): Promise<Ravel> {return this.$callMethod('ravelCast');}
  async redistributeHandles(): Promise<void> {return this.$callMethod('redistributeHandles');}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resortHandleIfDynamic(): Promise<void> {return this.$callMethod('resortHandleIfDynamic');}
  async selectedHandle(): Promise<number> {return this.$callMethod('selectedHandle');}
  async setDescription(a1: string): Promise<void> {return this.$callMethod('setDescription',a1);}
  async setDimension(...args: any[]): Promise<void> {return this.$callMethod('setDimension',...args);}
  async setDisplayFilterCaliper(a1: boolean): Promise<boolean> {return this.$callMethod('setDisplayFilterCaliper',a1);}
  async setHandleDescription(a1: number,a2: string): Promise<void> {return this.$callMethod('setHandleDescription',a1,a2);}
  async setHandleSortOrder(a1: string,a2: number): Promise<string> {return this.$callMethod('setHandleSortOrder',a1,a2);}
  async setRank(a1: number): Promise<void> {return this.$callMethod('setRank',a1);}
  async setSortOrder(a1: string): Promise<string> {return this.$callMethod('setSortOrder',a1);}
  async sortByValue(a1: string): Promise<void> {return this.$callMethod('sortByValue',a1);}
  async sortOrder(): Promise<string> {return this.$callMethod('sortOrder');}
  async toggleDisplayFilterCaliper(): Promise<boolean> {return this.$callMethod('toggleDisplayFilterCaliper');}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
}

export class RavelLockGroup extends CppClass {
  handleLockInfo: Sequence<HandleLockInfo>;
  constructor(prefix: string){
    super(prefix);
    this.handleLockInfo=new Sequence<HandleLockInfo>(this.$prefix()+'.handleLockInfo',HandleLockInfo);
  }
  async addHandleInfo(a1: Ravel): Promise<void> {return this.$callMethod('addHandleInfo',a1);}
  async addRavel(a1: Ravel): Promise<void> {return this.$callMethod('addRavel',a1);}
  async allLockHandles(): Promise<string[]> {return this.$callMethod('allLockHandles');}
  async broadcast(a1: Ravel): Promise<void> {return this.$callMethod('broadcast',a1);}
  async colour(): Promise<number> {return this.$callMethod('colour');}
  async handleNames(a1: number): Promise<string[]> {return this.$callMethod('handleNames',a1);}
  async initialBroadcast(): Promise<void> {return this.$callMethod('initialBroadcast');}
  async ravelNames(): Promise<string[]> {return this.$callMethod('ravelNames');}
  async ravels(): Promise<Sequence<Ravel>> {return this.$callMethod('ravels');}
  async removeFromGroup(a1: Ravel): Promise<void> {return this.$callMethod('removeFromGroup',a1);}
  async setLockHandles(a1: string[]): Promise<void> {return this.$callMethod('setLockHandles',a1);}
  async validateLockHandleInfo(): Promise<void> {return this.$callMethod('validateLockHandleInfo');}
}

export class RavelPopup extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.item=new Item(this.$prefix()+'.item');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseLeave(): Promise<void> {return this.$callMethod('mouseLeave');}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseOver(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseOver',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class SVGRenderer extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async height(): Promise<number> {return this.$callMethod('height');}
  async render(a1: minsky__dummy): Promise<void> {return this.$callMethod('render',a1);}
  async setResource(a1: string): Promise<void> {return this.$callMethod('setResource',a1);}
  async width(): Promise<number> {return this.$callMethod('width');}
}

export class Selection extends CppClass {
  bb: BoundingBox;
  bookmarks: Container<Bookmark>;
  createdIOvariables: Sequence<VariableBase>;
  groups: Sequence<Group>;
  inVariables: Sequence<VariableBase>;
  items: Sequence<Item>;
  outVariables: Sequence<VariableBase>;
  svgRenderer: SVGRenderer;
  wires: Sequence<Wire>;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.$prefix()+'.bb');
    this.bookmarks=new Container<Bookmark>(this.$prefix()+'.bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.$prefix()+'.createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.$prefix()+'.groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.$prefix()+'.inVariables',VariableBase);
    this.items=new Sequence<Item>(this.$prefix()+'.items',Item);
    this.outVariables=new Sequence<VariableBase>(this.$prefix()+'.outVariables',VariableBase);
    this.svgRenderer=new SVGRenderer(this.$prefix()+'.svgRenderer');
    this.wires=new Sequence<Wire>(this.$prefix()+'.wires',Wire);
  }
  async RESTProcess(a1: classdesc__RESTProcess_t,a2: string): Promise<void> {return this.$callMethod('RESTProcess',a1,a2);}
  async accessibleVars(): Promise<string[]> {return this.$callMethod('accessibleVars');}
  async addBookmark(a1: string): Promise<void> {return this.$callMethod('addBookmark',a1);}
  async addBookmarkXY(a1: number,a2: number,a3: string): Promise<void> {return this.$callMethod('addBookmarkXY',a1,a2,a3);}
  async addGroup(a1: Group): Promise<object> {return this.$callMethod('addGroup',a1);}
  async addInputVar(): Promise<void> {return this.$callMethod('addInputVar');}
  async addItem(a1: Item,a2: boolean): Promise<object> {return this.$callMethod('addItem',a1,a2);}
  async addOutputVar(): Promise<void> {return this.$callMethod('addOutputVar');}
  async addWire(...args: any[]): Promise<object> {return this.$callMethod('addWire',...args);}
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async adjustWiresGroup(a1: Wire): Promise<void> {return this.$callMethod('adjustWiresGroup',a1);}
  async arguments(): Promise<string> {return this.$callMethod('arguments');}
  async autoLayout(): Promise<void> {return this.$callMethod('autoLayout');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async bookmarkId(): Promise<string> {return this.$callMethod('bookmarkId');}
  async bookmarkList(): Promise<string[]> {return this.$callMethod('bookmarkList');}
  async bottom(): Promise<number> {return this.$callMethod('bottom');}
  async cBounds(): Promise<number[]> {return this.$callMethod('cBounds');}
  async checkAddIORegion(a1: Item): Promise<void> {return this.$callMethod('checkAddIORegion',a1);}
  async checkUnits(): Promise<object> {return this.$callMethod('checkUnits');}
  async classType(): Promise<string> {return this.$callMethod('classType');}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async clone(): Promise<Item> {return this.$callMethod('clone');}
  async closestInPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestInPort',a1,a2);}
  async closestOutPort(a1: number,a2: number): Promise<object> {return this.$callMethod('closestOutPort',a1,a2);}
  async computeDisplayZoom(): Promise<number> {return this.$callMethod('computeDisplayZoom');}
  async computeRelZoom(): Promise<void> {return this.$callMethod('computeRelZoom');}
  async contains(...args: any[]): Promise<boolean> {return this.$callMethod('contains',...args);}
  async copy(): Promise<object> {return this.$callMethod('copy');}
  async copyUnowned(): Promise<object> {return this.$callMethod('copyUnowned');}
  async corners(): Promise<object[]> {return this.$callMethod('corners');}
  async defaultExtension(): Promise<string> {return this.$callMethod('defaultExtension');}
  async deleteAttachedWires(): Promise<void> {return this.$callMethod('deleteAttachedWires');}
  async deleteBookmark(a1: number): Promise<void> {return this.$callMethod('deleteBookmark',a1);}
  async deleteCallback(...args: string[]): Promise<string> {return this.$callMethod('deleteCallback',...args);}
  async deleteItem(a1: Item): Promise<void> {return this.$callMethod('deleteItem',a1);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async disableDelayedTooltip(): Promise<void> {return this.$callMethod('disableDelayedTooltip');}
  async displayContents(): Promise<boolean> {return this.$callMethod('displayContents');}
  async displayContentsChanged(): Promise<boolean> {return this.$callMethod('displayContentsChanged');}
  async displayDelayedTooltip(a1: number,a2: number): Promise<void> {return this.$callMethod('displayDelayedTooltip',a1,a2);}
  async displayTooltip(a1: minsky__dummy,a2: string): Promise<void> {return this.$callMethod('displayTooltip',a1,a2);}
  async displayZoom(...args: number[]): Promise<number> {return this.$callMethod('displayZoom',...args);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async draw1edge(a1: Sequence<VariableBase>,a2: minsky__dummy,a3: number): Promise<void> {return this.$callMethod('draw1edge',a1,a2,a3);}
  async drawEdgeVariables(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawEdgeVariables',a1);}
  async drawIORegion(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawIORegion',a1);}
  async drawPorts(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawPorts',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async drawSelected(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawSelected',a1);}
  async dummyDraw(): Promise<void> {return this.$callMethod('dummyDraw');}
  async edgeScale(): Promise<number> {return this.$callMethod('edgeScale');}
  async editorMode(): Promise<boolean> {return this.$callMethod('editorMode');}
  async empty(): Promise<boolean> {return this.$callMethod('empty');}
  async ensureBBValid(): Promise<void> {return this.$callMethod('ensureBBValid');}
  async ensureGroupInserted(a1: Group): Promise<void> {return this.$callMethod('ensureGroupInserted',a1);}
  async ensureItemInserted(a1: Item): Promise<void> {return this.$callMethod('ensureItemInserted',a1);}
  async findGroup(a1: Group): Promise<object> {return this.$callMethod('findGroup',a1);}
  async findItem(a1: Item): Promise<object> {return this.$callMethod('findItem',a1);}
  async findWire(a1: Wire): Promise<object> {return this.$callMethod('findWire',a1);}
  async flip(): Promise<void> {return this.$callMethod('flip');}
  async flipContents(): Promise<void> {return this.$callMethod('flipContents');}
  async formula(): Promise<string> {return this.$callMethod('formula');}
  async globalGroup(): Promise<Group> {return this.$callMethod('globalGroup');}
  async gotoBookmark(a1: number): Promise<void> {return this.$callMethod('gotoBookmark',a1);}
  async gotoBookmark_b(a1: Bookmark): Promise<void> {return this.$callMethod('gotoBookmark_b',a1);}
  async height(): Promise<number> {return this.$callMethod('height');}
  async higher(a1: Group): Promise<boolean> {return this.$callMethod('higher',a1);}
  async iHeight(...args: any[]): Promise<number> {return this.$callMethod('iHeight',...args);}
  async iWidth(...args: any[]): Promise<number> {return this.$callMethod('iWidth',...args);}
  async id(): Promise<string> {return this.$callMethod('id');}
  async inIORegion(a1: number,a2: number): Promise<string> {return this.$callMethod('inIORegion',a1,a2);}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async insertGroup(a1: Group): Promise<void> {return this.$callMethod('insertGroup',a1);}
  async insertItem(a1: Item): Promise<void> {return this.$callMethod('insertItem',a1);}
  async ioVar(): Promise<boolean> {return this.$callMethod('ioVar');}
  async itemPtrFromThis(): Promise<object> {return this.$callMethod('itemPtrFromThis');}
  async json_pack(a1: classdesc__json_pack_t): Promise<void> {return this.$callMethod('json_pack',a1);}
  async left(): Promise<number> {return this.$callMethod('left');}
  async level(): Promise<number> {return this.$callMethod('level');}
  async localZoom(): Promise<number> {return this.$callMethod('localZoom');}
  async m_sf(...args: number[]): Promise<number> {return this.$callMethod('m_sf',...args);}
  async m_x(...args: number[]): Promise<number> {return this.$callMethod('m_x',...args);}
  async m_y(...args: number[]): Promise<number> {return this.$callMethod('m_y',...args);}
  async makeSubroutine(): Promise<void> {return this.$callMethod('makeSubroutine');}
  async margins(a1: number,a2: number): Promise<void> {return this.$callMethod('margins',a1,a2);}
  async marginsV(): Promise<number[]> {return this.$callMethod('marginsV');}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveContents(a1: Group): Promise<void> {return this.$callMethod('moveContents',a1);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async name(): Promise<string> {return this.$callMethod('name');}
  async nocycles(): Promise<boolean> {return this.$callMethod('nocycles');}
  async normaliseGroupRefs(a1: Group): Promise<void> {return this.$callMethod('normaliseGroupRefs',a1);}
  async numGroups(): Promise<number> {return this.$callMethod('numGroups');}
  async numItems(): Promise<number> {return this.$callMethod('numItems');}
  async numWires(): Promise<number> {return this.$callMethod('numWires');}
  async onBorder(...args: boolean[]): Promise<boolean> {return this.$callMethod('onBorder',...args);}
  async onItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onItem',a1,a2);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async onResizeHandles(...args: boolean[]): Promise<boolean> {return this.$callMethod('onResizeHandles',...args);}
  async portX(a1: number): Promise<number> {return this.$callMethod('portX',a1);}
  async portY(a1: number): Promise<number> {return this.$callMethod('portY',a1);}
  async ports(a1: number): Promise<object> {return this.$callMethod('ports',a1);}
  async portsSize(): Promise<number> {return this.$callMethod('portsSize');}
  async randomLayout(): Promise<void> {return this.$callMethod('randomLayout');}
  async relZoom(...args: number[]): Promise<number> {return this.$callMethod('relZoom',...args);}
  async removeDisplayPlot(): Promise<void> {return this.$callMethod('removeDisplayPlot');}
  async removeGroup(a1: Group): Promise<object> {return this.$callMethod('removeGroup',a1);}
  async removeItem(a1: Item): Promise<object> {return this.$callMethod('removeItem',a1);}
  async removeWire(a1: Wire): Promise<object> {return this.$callMethod('removeWire',a1);}
  async renameAllInstances(a1: string,a2: string): Promise<void> {return this.$callMethod('renameAllInstances',a1,a2);}
  async resize(a1: LassoBox): Promise<void> {return this.$callMethod('resize',a1);}
  async resizeHandleSize(): Promise<number> {return this.$callMethod('resizeHandleSize');}
  async resizeOnContents(): Promise<void> {return this.$callMethod('resizeOnContents');}
  async right(): Promise<number> {return this.$callMethod('right');}
  async rotFactor(): Promise<number> {return this.$callMethod('rotFactor');}
  async rotation(...args: any[]): Promise<number> {return this.$callMethod('rotation',...args);}
  async rotationAsRadians(): Promise<object> {return this.$callMethod('rotationAsRadians');}
  async scaleFactor(...args: any[]): Promise<number> {return this.$callMethod('scaleFactor',...args);}
  async select(a1: number,a2: number): Promise<object> {return this.$callMethod('select',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async setZoom(a1: number): Promise<void> {return this.$callMethod('setZoom',a1);}
  async splitBoundaryCrossingWires(): Promise<void> {return this.$callMethod('splitBoundaryCrossingWires');}
  async summariseGodleys(): Promise<object[]> {return this.$callMethod('summariseGodleys');}
  async throw_error(a1: string): Promise<void> {return this.$callMethod('throw_error',a1);}
  async title(...args: string[]): Promise<string> {return this.$callMethod('title',...args);}
  async toggleEditorMode(): Promise<void> {return this.$callMethod('toggleEditorMode');}
  async toggleItemMembership(a1: Item): Promise<void> {return this.$callMethod('toggleItemMembership',a1);}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async top(): Promise<number> {return this.$callMethod('top');}
  async uniqueItems(...args: any[]): Promise<boolean> {return this.$callMethod('uniqueItems',...args);}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async updateIcon(a1: number): Promise<void> {return this.$callMethod('updateIcon',a1);}
  async value(): Promise<number> {return this.$callMethod('value');}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
  async width(): Promise<number> {return this.$callMethod('width');}
  async x(): Promise<number> {return this.$callMethod('x');}
  async y(): Promise<number> {return this.$callMethod('y');}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class Sheet extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
  }
  async clickType(a1: number,a2: number): Promise<string> {return this.$callMethod('clickType',a1,a2);}
  async computeValue(): Promise<void> {return this.$callMethod('computeValue');}
  async contains(a1: number,a2: number): Promise<boolean> {return this.$callMethod('contains',a1,a2);}
  async corners(): Promise<object[]> {return this.$callMethod('corners');}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async drawResizeHandles(a1: minsky__dummy): Promise<void> {return this.$callMethod('drawResizeHandles',a1);}
  async exportAsCSV(a1: string,a2: boolean): Promise<void> {return this.$callMethod('exportAsCSV',a1,a2);}
  async inItem(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inItem',a1,a2);}
  async inRavel(a1: number,a2: number): Promise<boolean> {return this.$callMethod('inRavel',a1,a2);}
  async onKeyPress(a1: number,a2: string,a3: number): Promise<boolean> {return this.$callMethod('onKeyPress',a1,a2,a3);}
  async onRavelButton(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onRavelButton',a1,a2);}
  async onResizeHandle(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onResizeHandle',a1,a2);}
  async scrollDown(): Promise<boolean> {return this.$callMethod('scrollDown');}
  async scrollUp(): Promise<boolean> {return this.$callMethod('scrollUp');}
  async setSliceIndicator(): Promise<void> {return this.$callMethod('setSliceIndicator');}
  async showColSlice(...args: string[]): Promise<string> {return this.$callMethod('showColSlice',...args);}
  async showRavel(...args: boolean[]): Promise<boolean> {return this.$callMethod('showRavel',...args);}
  async showRowSlice(...args: string[]): Promise<string> {return this.$callMethod('showRowSlice',...args);}
}

export class SwitchIcon extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
  }
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async flip(): Promise<void> {return this.$callMethod('flip');}
  async flipped(...args: boolean[]): Promise<boolean> {return this.$callMethod('flipped',...args);}
  async numCases(): Promise<number> {return this.$callMethod('numCases');}
  async setNumCases(a1: number): Promise<void> {return this.$callMethod('setNumCases',a1);}
  async switchIconCast(): Promise<SwitchIcon> {return this.$callMethod('switchIconCast');}
  async switchValue(): Promise<number> {return this.$callMethod('switchValue');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async value(): Promise<number> {return this.$callMethod('value');}
}

export class Units extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
  }
  async latexStr(): Promise<string> {return this.$callMethod('latexStr');}
  async normalise(): Promise<void> {return this.$callMethod('normalise');}
  async str(): Promise<string> {return this.$callMethod('str');}
}

export class UserFunction extends Item {
  argNames: Sequence<string>;
  argVals: Sequence<number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
    this.argNames=new Sequence<string>(this.$prefix()+'.argNames');
    this.argVals=new Sequence<number>(this.$prefix()+'.argVals');
  }
  async compile(): Promise<void> {return this.$callMethod('compile');}
  async create(a1: string): Promise<UserFunction> {return this.$callMethod('create',a1);}
  async description(...args: any[]): Promise<string> {return this.$callMethod('description',...args);}
  async displayTooltip(a1: minsky__dummy,a2: string): Promise<void> {return this.$callMethod('displayTooltip',a1,a2);}
  async draw(a1: minsky__dummy): Promise<void> {return this.$callMethod('draw',a1);}
  async evaluate(a1: number,a2: number): Promise<number> {return this.$callMethod('evaluate',a1,a2);}
  async expression(...args: string[]): Promise<string> {return this.$callMethod('expression',...args);}
  async name(): Promise<string> {return this.$callMethod('name');}
  async nextId(...args: number[]): Promise<number> {return this.$callMethod('nextId',...args);}
  async symbolNames(): Promise<string[]> {return this.$callMethod('symbolNames');}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
}

export class VariableInstanceList extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async gotoInstance(a1: number): Promise<void> {return this.$callMethod('gotoInstance',a1);}
  async names(): Promise<string[]> {return this.$callMethod('names');}
}

export class VariablePane extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  bottomRowMargin: Sequence<number>;
  item: Item;
  rightColMargin: Sequence<number>;
  selection: Container<string>;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.$prefix()+'.backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.$prefix()+'.bottomRowMargin');
    this.item=new Item(this.$prefix()+'.item');
    this.rightColMargin=new Sequence<number>(this.$prefix()+'.rightColMargin');
    this.selection=new Container<string>(this.$prefix()+'.selection');
    this.wire=new Wire(this.$prefix()+'.wire');
  }
  async cell(a1: number,a2: number): Promise<VariablePaneCell> {return this.$callMethod('cell',a1,a2);}
  async colX(a1: number): Promise<number> {return this.$callMethod('colX',a1);}
  async controlMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('controlMouseDown',a1,a2);}
  async deselect(a1: string): Promise<void> {return this.$callMethod('deselect',a1);}
  async destroyFrame(): Promise<void> {return this.$callMethod('destroyFrame');}
  async draw(): Promise<void> {return this.$callMethod('draw');}
  async evenHeight(): Promise<boolean> {return this.$callMethod('evenHeight');}
  async frameArgs(): Promise<minsky__RenderNativeWindow__RenderFrameArgs> {return this.$callMethod('frameArgs');}
  async getItemAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getItemAt',a1,a2);}
  async getWireAt(a1: number,a2: number): Promise<boolean> {return this.$callMethod('getWireAt',a1,a2);}
  async hasScrollBars(): Promise<boolean> {return this.$callMethod('hasScrollBars');}
  async init(): Promise<void> {return this.$callMethod('init');}
  async justification(a1: number): Promise<string> {return this.$callMethod('justification',a1);}
  async keyPress(a1: minsky__EventInterface__KeyPressArgs): Promise<boolean> {return this.$callMethod('keyPress',a1);}
  async m_zoomFactor(...args: number[]): Promise<number> {return this.$callMethod('m_zoomFactor',...args);}
  async mouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseDown',a1,a2);}
  async mouseMove(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseMove',a1,a2);}
  async mouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('mouseUp',a1,a2);}
  async moveCursorTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveCursorTo',a1,a2);}
  async moveTo(a1: number,a2: number): Promise<void> {return this.$callMethod('moveTo',a1,a2);}
  async numCols(): Promise<number> {return this.$callMethod('numCols');}
  async numRows(): Promise<number> {return this.$callMethod('numRows');}
  async offsx(...args: number[]): Promise<number> {return this.$callMethod('offsx',...args);}
  async offsy(...args: number[]): Promise<number> {return this.$callMethod('offsy',...args);}
  async position(): Promise<number[]> {return this.$callMethod('position');}
  async registerImage(): Promise<void> {return this.$callMethod('registerImage');}
  async renderFrame(a1: minsky__RenderNativeWindow__RenderFrameArgs): Promise<void> {return this.$callMethod('renderFrame',a1);}
  async renderToEMF(a1: string): Promise<void> {return this.$callMethod('renderToEMF',a1);}
  async renderToPDF(a1: string): Promise<void> {return this.$callMethod('renderToPDF',a1);}
  async renderToPNG(a1: string): Promise<void> {return this.$callMethod('renderToPNG',a1);}
  async renderToPS(a1: string): Promise<void> {return this.$callMethod('renderToPS',a1);}
  async renderToSVG(a1: string): Promise<void> {return this.$callMethod('renderToSVG',a1);}
  async reportDrawTime(a1: number): Promise<void> {return this.$callMethod('reportDrawTime',a1);}
  async requestRedraw(): Promise<void> {return this.$callMethod('requestRedraw');}
  async resolutionScaleFactor(...args: any[]): Promise<number> {return this.$callMethod('resolutionScaleFactor',...args);}
  async rowY(a1: number): Promise<number> {return this.$callMethod('rowY',a1);}
  async scaleFactor(): Promise<number> {return this.$callMethod('scaleFactor');}
  async select(a1: string): Promise<void> {return this.$callMethod('select',a1);}
  async shift(...args: boolean[]): Promise<boolean> {return this.$callMethod('shift',...args);}
  async typeName(a1: number): Promise<string> {return this.$callMethod('typeName',a1);}
  async update(): Promise<void> {return this.$callMethod('update');}
  async updateWithHeight(a1: number): Promise<void> {return this.$callMethod('updateWithHeight',a1);}
  async zoom(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('zoom',a1,a2,a3);}
  async zoomFactor(): Promise<number> {return this.$callMethod('zoomFactor');}
}

export class VariablePaneCell extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async emplace(): Promise<void> {return this.$callMethod('emplace');}
  async height(): Promise<number> {return this.$callMethod('height');}
  async reset(a1: minsky__dummy): Promise<void> {return this.$callMethod('reset',a1);}
  async show(): Promise<void> {return this.$callMethod('show');}
  async variable(): Promise<VariableBase> {return this.$callMethod('variable');}
  async width(): Promise<number> {return this.$callMethod('width');}
}

export class VariableValue extends CppClass {
  csvDialog: CSVDialog;
  rhs: civita__ITensor;
  tensorInit: civita__TensorVal;
  units: Units;
  constructor(prefix: string){
    super(prefix);
    this.csvDialog=new CSVDialog(this.$prefix()+'.csvDialog');
    this.rhs=new civita__ITensor(this.$prefix()+'.rhs');
    this.tensorInit=new civita__TensorVal(this.$prefix()+'.tensorInit');
    this.units=new Units(this.$prefix()+'.units');
  }
  async adjustSliderBounds(): Promise<void> {return this.$callMethod('adjustSliderBounds');}
  async allocValue(): Promise<VariableValue> {return this.$callMethod('allocValue');}
  async at(a1: number): Promise<number> {return this.$callMethod('at',a1);}
  async atHCIndex(a1: number): Promise<number> {return this.$callMethod('atHCIndex',a1);}
  async begin(): Promise<number> {return this.$callMethod('begin');}
  async cancel(a1: boolean): Promise<void> {return this.$callMethod('cancel',a1);}
  async data(): Promise<number[]> {return this.$callMethod('data');}
  async detailedText(...args: string[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async enableSlider(...args: boolean[]): Promise<boolean> {return this.$callMethod('enableSlider',...args);}
  async end(): Promise<number> {return this.$callMethod('end');}
  async exportAsCSV(a1: string,a2: string,a3: boolean): Promise<void> {return this.$callMethod('exportAsCSV',a1,a2,a3);}
  async godleyOverridden(...args: boolean[]): Promise<boolean> {return this.$callMethod('godleyOverridden',...args);}
  async hypercube(...args: any[]): Promise<civita__Hypercube> {return this.$callMethod('hypercube',...args);}
  async idx(): Promise<number> {return this.$callMethod('idx');}
  async idxInRange(): Promise<boolean> {return this.$callMethod('idxInRange');}
  async imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): Promise<void> {return this.$callMethod('imposeDimensions',a1);}
  async incrSlider(a1: number): Promise<void> {return this.$callMethod('incrSlider',a1);}
  async index(...args: any[]): Promise<civita__Index> {return this.$callMethod('index',...args);}
  async init(...args: any[]): Promise<string> {return this.$callMethod('init',...args);}
  async isFlowVar(): Promise<boolean> {return this.$callMethod('isFlowVar');}
  async isZero(): Promise<boolean> {return this.$callMethod('isZero');}
  async lhs(): Promise<boolean> {return this.$callMethod('lhs');}
  async maxSliderSteps(): Promise<number> {return this.$callMethod('maxSliderSteps');}
  async name(...args: string[]): Promise<string> {return this.$callMethod('name',...args);}
  async rank(): Promise<number> {return this.$callMethod('rank');}
  async reset_idx(): Promise<void> {return this.$callMethod('reset_idx');}
  async setArgument(a1: civita__ITensor,a2: civita__ITensor__Args): Promise<void> {return this.$callMethod('setArgument',a1,a2);}
  async setArguments(...args: any[]): Promise<void> {return this.$callMethod('setArguments',...args);}
  async setUnits(a1: string): Promise<void> {return this.$callMethod('setUnits',a1);}
  async setValue(...args: any[]): Promise<number> {return this.$callMethod('setValue',...args);}
  async shape(): Promise<number[]> {return this.$callMethod('shape');}
  async size(): Promise<number> {return this.$callMethod('size');}
  async sliderMax(...args: number[]): Promise<number> {return this.$callMethod('sliderMax',...args);}
  async sliderMin(...args: number[]): Promise<number> {return this.$callMethod('sliderMin',...args);}
  async sliderSet(a1: number): Promise<void> {return this.$callMethod('sliderSet',a1);}
  async sliderStep(...args: number[]): Promise<number> {return this.$callMethod('sliderStep',...args);}
  async sliderStepRel(...args: boolean[]): Promise<boolean> {return this.$callMethod('sliderStepRel',...args);}
  async summary(): Promise<object> {return this.$callMethod('summary');}
  async temp(): Promise<boolean> {return this.$callMethod('temp');}
  async tooltip(...args: string[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async type(): Promise<string> {return this.$callMethod('type');}
  async typeName(a1: number): Promise<string> {return this.$callMethod('typeName',a1);}
  async unitsCached(...args: boolean[]): Promise<boolean> {return this.$callMethod('unitsCached',...args);}
  async value(): Promise<number> {return this.$callMethod('value');}
  async valueAt(a1: number): Promise<number> {return this.$callMethod('valueAt',a1);}
  async valueId(): Promise<string> {return this.$callMethod('valueId');}
}

export class VariableValues extends Map<string,VariableValue> {
  constructor(prefix: string|Map<string,VariableValue>){
    if (typeof prefix==='string')
      super(prefix,VariableValue)
    else
      super(prefix.$prefix(),VariableValue)
  }
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async initValue(...args: any[]): Promise<object> {return this.$callMethod('initValue',...args);}
  async newName(a1: string): Promise<string> {return this.$callMethod('newName',a1);}
  async reset(): Promise<void> {return this.$callMethod('reset');}
  async resetUnitsCache(): Promise<void> {return this.$callMethod('resetUnitsCache');}
  async resetValue(a1: VariableValue): Promise<void> {return this.$callMethod('resetValue',a1);}
  async summarise(): Promise<object[]> {return this.$callMethod('summarise');}
  async validEntries(): Promise<boolean> {return this.$callMethod('validEntries');}
}

export class Wire extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async adjustBookmark(): Promise<void> {return this.$callMethod('adjustBookmark');}
  async bookmark(...args: boolean[]): Promise<boolean> {return this.$callMethod('bookmark',...args);}
  async coords(...args: any[]): Promise<number[]> {return this.$callMethod('coords',...args);}
  async deleteHandle(a1: number,a2: number): Promise<void> {return this.$callMethod('deleteHandle',a1,a2);}
  async detailedText(...args: any[]): Promise<string> {return this.$callMethod('detailedText',...args);}
  async draw(a1: minsky__dummy,a2: boolean): Promise<void> {return this.$callMethod('draw',a1,a2);}
  async editHandle(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('editHandle',a1,a2,a3);}
  async from(): Promise<object> {return this.$callMethod('from');}
  async insertHandle(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('insertHandle',a1,a2,a3);}
  async mouseFocus(...args: boolean[]): Promise<boolean> {return this.$callMethod('mouseFocus',...args);}
  async moveIntoGroup(a1: Group): Promise<void> {return this.$callMethod('moveIntoGroup',a1);}
  async moveToPorts(a1: Port,a2: Port): Promise<void> {return this.$callMethod('moveToPorts',a1,a2);}
  async near(a1: number,a2: number): Promise<boolean> {return this.$callMethod('near',a1,a2);}
  async nearestHandle(a1: number,a2: number): Promise<number> {return this.$callMethod('nearestHandle',a1,a2);}
  async selected(...args: boolean[]): Promise<boolean> {return this.$callMethod('selected',...args);}
  async split(): Promise<void> {return this.$callMethod('split');}
  async storeCairoCoords(a1: minsky__dummy): Promise<void> {return this.$callMethod('storeCairoCoords',a1);}
  async straighten(): Promise<void> {return this.$callMethod('straighten');}
  async to(): Promise<object> {return this.$callMethod('to');}
  async tooltip(...args: any[]): Promise<string> {return this.$callMethod('tooltip',...args);}
  async units(a1: boolean): Promise<object> {return this.$callMethod('units',a1);}
  async updateBoundingBox(): Promise<void> {return this.$callMethod('updateBoundingBox');}
  async visible(): Promise<boolean> {return this.$callMethod('visible');}
}

export class civita__Conversions extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.$prefix())
  }
  async convert(a1: number,a2: string,a3: string): Promise<number> {return this.$callMethod('convert',a1,a2,a3);}
}

export class civita__Dimension extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async type(...args: string[]): Promise<string> {return this.$callMethod('type',...args);}
  async units(...args: string[]): Promise<string> {return this.$callMethod('units',...args);}
}

export class civita__Hypercube extends CppClass {
  xvectors: Sequence<civita__XVector>;
  constructor(prefix: string){
    super(prefix);
    this.xvectors=new Sequence<civita__XVector>(this.$prefix()+'.xvectors',civita__XVector);
  }
  async dimLabels(): Promise<string[]> {return this.$callMethod('dimLabels');}
  async dims(...args: any[]): Promise<number[]> {return this.$callMethod('dims',...args);}
  async dimsAreDistinct(): Promise<boolean> {return this.$callMethod('dimsAreDistinct');}
  async fromJson(a1: string): Promise<object> {return this.$callMethod('fromJson',a1);}
  async json(): Promise<string> {return this.$callMethod('json');}
  async logNumElements(): Promise<number> {return this.$callMethod('logNumElements');}
  async numElements(): Promise<number> {return this.$callMethod('numElements');}
  async rank(): Promise<number> {return this.$callMethod('rank');}
  async splitIndex(a1: number): Promise<number[]> {return this.$callMethod('splitIndex',a1);}
}

export class civita__ITensor extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async at(a1: number): Promise<number> {return this.$callMethod('at',a1);}
  async atHCIndex(a1: number): Promise<number> {return this.$callMethod('atHCIndex',a1);}
  async cancel(a1: boolean): Promise<void> {return this.$callMethod('cancel',a1);}
  async data(): Promise<number[]> {return this.$callMethod('data');}
  async hypercube(...args: any[]): Promise<civita__Hypercube> {return this.$callMethod('hypercube',...args);}
  async imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): Promise<void> {return this.$callMethod('imposeDimensions',a1);}
  async index(): Promise<civita__Index> {return this.$callMethod('index');}
  async rank(): Promise<number> {return this.$callMethod('rank');}
  async setArgument(a1: civita__ITensor,a2: civita__ITensor__Args): Promise<void> {return this.$callMethod('setArgument',a1,a2);}
  async setArguments(...args: any[]): Promise<void> {return this.$callMethod('setArguments',...args);}
  async shape(): Promise<number[]> {return this.$callMethod('shape');}
  async size(): Promise<number> {return this.$callMethod('size');}
}

export class civita__Index extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async begin(): Promise<object> {return this.$callMethod('begin');}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async empty(): Promise<boolean> {return this.$callMethod('empty');}
  async end(): Promise<object> {return this.$callMethod('end');}
  async linealOffset(a1: number): Promise<number> {return this.$callMethod('linealOffset',a1);}
  async size(): Promise<number> {return this.$callMethod('size');}
}

export class civita__NamedDimension extends CppClass {
  dimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dimension=new civita__Dimension(this.$prefix()+'.dimension');
  }
  async name(...args: string[]): Promise<string> {return this.$callMethod('name',...args);}
}

export class civita__TensorVal extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async allocVal(): Promise<void> {return this.$callMethod('allocVal');}
  async assign(a1: Map<number,number>): Promise<void> {return this.$callMethod('assign',a1);}
  async at(a1: number): Promise<number> {return this.$callMethod('at',a1);}
  async atHCIndex(a1: number): Promise<number> {return this.$callMethod('atHCIndex',a1);}
  async begin(): Promise<number> {return this.$callMethod('begin');}
  async cancel(a1: boolean): Promise<void> {return this.$callMethod('cancel',a1);}
  async data(): Promise<number[]> {return this.$callMethod('data');}
  async end(): Promise<number> {return this.$callMethod('end');}
  async hypercube(...args: any[]): Promise<civita__Hypercube> {return this.$callMethod('hypercube',...args);}
  async imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): Promise<void> {return this.$callMethod('imposeDimensions',a1);}
  async index(...args: any[]): Promise<civita__Index> {return this.$callMethod('index',...args);}
  async rank(): Promise<number> {return this.$callMethod('rank');}
  async setArgument(a1: civita__ITensor,a2: civita__ITensor__Args): Promise<void> {return this.$callMethod('setArgument',a1,a2);}
  async setArguments(...args: any[]): Promise<void> {return this.$callMethod('setArguments',...args);}
  async setDimensions(a1: number[]): Promise<void> {return this.$callMethod('setDimensions',a1);}
  async setHypercube(a1: civita__Hypercube): Promise<void> {return this.$callMethod('setHypercube',a1);}
  async shape(): Promise<number[]> {return this.$callMethod('shape');}
  async size(): Promise<number> {return this.$callMethod('size');}
  async updateTimestamp(): Promise<void> {return this.$callMethod('updateTimestamp');}
}

export class civita__XVector extends CppClass {
  dimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dimension=new civita__Dimension(this.$prefix()+'.dimension');
  }
  async checkThisType(): Promise<boolean> {return this.$callMethod('checkThisType');}
  async imposeDimension(): Promise<void> {return this.$callMethod('imposeDimension');}
  async name(...args: string[]): Promise<string> {return this.$callMethod('name',...args);}
  async push_back(...args: any[]): Promise<void> {return this.$callMethod('push_back',...args);}
  async timeFormat(): Promise<string> {return this.$callMethod('timeFormat');}
}

export class ecolab__Plot__LineStyle extends CppClass {
  colour: ecolab__cairo__Colour;
  constructor(prefix: string){
    super(prefix);
    this.colour=new ecolab__cairo__Colour(this.$prefix()+'.colour');
  }
  async barWidth(...args: number[]): Promise<number> {return this.$callMethod('barWidth',...args);}
  async dashPattern(): Promise<number[]> {return this.$callMethod('dashPattern');}
  async dashStyle(...args: string[]): Promise<string> {return this.$callMethod('dashStyle',...args);}
  async plotType(...args: string[]): Promise<string> {return this.$callMethod('plotType',...args);}
  async width(...args: number[]): Promise<number> {return this.$callMethod('width',...args);}
}

export class ecolab__cairo__Colour extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async a(...args: number[]): Promise<number> {return this.$callMethod('a',...args);}
  async b(...args: number[]): Promise<number> {return this.$callMethod('b',...args);}
  async g(...args: number[]): Promise<number> {return this.$callMethod('g',...args);}
  async r(...args: number[]): Promise<number> {return this.$callMethod('r',...args);}
}

export class minsky__Canvas__ZoomCrop extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async height(...args: number[]): Promise<number> {return this.$callMethod('height',...args);}
  async left(...args: number[]): Promise<number> {return this.$callMethod('left',...args);}
  async top(...args: number[]): Promise<number> {return this.$callMethod('top',...args);}
  async width(...args: number[]): Promise<number> {return this.$callMethod('width',...args);}
  async zoom(...args: number[]): Promise<number> {return this.$callMethod('zoom',...args);}
}

export class ravelCAPI__Database extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async close(): Promise<void> {return this.$callMethod('close');}
  async connect(a1: string,a2: string,a3: string): Promise<void> {return this.$callMethod('connect',a1,a2,a3);}
  async createTable(a1: string,a2: ravel__DataSpec): Promise<void> {return this.$callMethod('createTable',a1,a2);}
  async deduplicate(a1: string,a2: ravel__DataSpec): Promise<void> {return this.$callMethod('deduplicate',a1,a2);}
  async fullHypercube(a1: ravelCAPI__Ravel): Promise<void> {return this.$callMethod('fullHypercube',a1);}
  async hyperSlice(a1: ravelCAPI__Ravel): Promise<object> {return this.$callMethod('hyperSlice',a1);}
  async loadDatabase(a1: string[],a2: ravel__DataSpec): Promise<void> {return this.$callMethod('loadDatabase',a1,a2);}
  async numericalColumnNames(): Promise<string[]> {return this.$callMethod('numericalColumnNames');}
  async setAxisNames(a1: Container<string>,a2: string): Promise<void> {return this.$callMethod('setAxisNames',a1,a2);}
  async tableNames(): Promise<string[]> {return this.$callMethod('tableNames');}
}

export class ravelCAPI__Ravel extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  async addHandle(a1: string,a2: string[]): Promise<void> {return this.$callMethod('addHandle',a1,a2);}
  async adjustSlicer(a1: number): Promise<void> {return this.$callMethod('adjustSlicer',a1);}
  async allSliceLabels(a1: number,a2: string): Promise<string[]> {return this.$callMethod('allSliceLabels',a1,a2);}
  async applyCustomPermutation(a1: number,a2: number[]): Promise<void> {return this.$callMethod('applyCustomPermutation',a1,a2);}
  async available(): Promise<boolean> {return this.$callMethod('available');}
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async currentPermutation(a1: number): Promise<number[]> {return this.$callMethod('currentPermutation',a1);}
  async daysUntilExpired(): Promise<number> {return this.$callMethod('daysUntilExpired');}
  async description(...args: any[]): Promise<string> {return this.$callMethod('description',...args);}
  async displayFilterCaliper(a1: number,a2: boolean): Promise<void> {return this.$callMethod('displayFilterCaliper',a1,a2);}
  async explain(a1: number,a2: number): Promise<string> {return this.$callMethod('explain',a1,a2);}
  async fromXML(a1: string): Promise<void> {return this.$callMethod('fromXML',a1);}
  async getCaliperPositions(a1: number): Promise<object> {return this.$callMethod('getCaliperPositions',a1);}
  async getHandleState(a1: number): Promise<object> {return this.$callMethod('getHandleState',a1);}
  async getRavelState(): Promise<object> {return this.$callMethod('getRavelState');}
  async handleDescription(a1: number): Promise<string> {return this.$callMethod('handleDescription',a1);}
  async handleSetReduction(a1: number,a2: string): Promise<void> {return this.$callMethod('handleSetReduction',a1,a2);}
  async hyperSlice(a1: civita__ITensor): Promise<object> {return this.$callMethod('hyperSlice',a1);}
  async lastError(): Promise<string> {return this.$callMethod('lastError');}
  async nextReduction(a1: string): Promise<void> {return this.$callMethod('nextReduction',a1);}
  async numAllSliceLabels(a1: number): Promise<number> {return this.$callMethod('numAllSliceLabels',a1);}
  async numHandles(): Promise<number> {return this.$callMethod('numHandles');}
  async numSliceLabels(a1: number): Promise<number> {return this.$callMethod('numSliceLabels',a1);}
  async onMouseDown(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseDown',a1,a2);}
  async onMouseLeave(): Promise<void> {return this.$callMethod('onMouseLeave');}
  async onMouseMotion(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseMotion',a1,a2);}
  async onMouseOver(a1: number,a2: number): Promise<boolean> {return this.$callMethod('onMouseOver',a1,a2);}
  async onMouseUp(a1: number,a2: number): Promise<void> {return this.$callMethod('onMouseUp',a1,a2);}
  async orderLabels(a1: number,a2: string): Promise<void> {return this.$callMethod('orderLabels',a1,a2);}
  async outputHandleIds(): Promise<number[]> {return this.$callMethod('outputHandleIds');}
  async populateFromHypercube(a1: civita__Hypercube): Promise<void> {return this.$callMethod('populateFromHypercube',a1);}
  async radius(): Promise<number> {return this.$callMethod('radius');}
  async rank(): Promise<number> {return this.$callMethod('rank');}
  async redistributeHandles(): Promise<void> {return this.$callMethod('redistributeHandles');}
  async render(a1: CAPIRenderer): Promise<void> {return this.$callMethod('render',a1);}
  async rescale(a1: number): Promise<void> {return this.$callMethod('rescale',a1);}
  async resetExplain(): Promise<void> {return this.$callMethod('resetExplain');}
  async selectedHandle(): Promise<number> {return this.$callMethod('selectedHandle');}
  async setCaliperPositions(a1: number,a2: number,a3: number): Promise<void> {return this.$callMethod('setCaliperPositions',a1,a2,a3);}
  async setCalipers(a1: number,a2: string,a3: string): Promise<void> {return this.$callMethod('setCalipers',a1,a2,a3);}
  async setExplain(a1: string,a2: number,a3: number): Promise<void> {return this.$callMethod('setExplain',a1,a2,a3);}
  async setHandleDescription(a1: number,a2: string): Promise<void> {return this.$callMethod('setHandleDescription',a1,a2);}
  async setHandleState(a1: number,a2: ravel__HandleState): Promise<void> {return this.$callMethod('setHandleState',a1,a2);}
  async setOutputHandleIds(a1: number[]): Promise<void> {return this.$callMethod('setOutputHandleIds',a1);}
  async setRavelState(a1: ravel__RavelState): Promise<void> {return this.$callMethod('setRavelState',a1);}
  async setSlicer(a1: number,a2: string): Promise<void> {return this.$callMethod('setSlicer',a1,a2);}
  async sliceLabels(a1: number): Promise<string[]> {return this.$callMethod('sliceLabels',a1);}
  async sortByValue(a1: civita__ITensor,a2: string): Promise<void> {return this.$callMethod('sortByValue',a1,a2);}
  async toXML(): Promise<string> {return this.$callMethod('toXML');}
  async version(): Promise<string> {return this.$callMethod('version');}
}

export class ravel__DataSpec extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensions: Sequence<civita__NamedDimension>;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(this.$prefix()+'.dataCols');
    this.dimensionCols=new Container<number>(this.$prefix()+'.dimensionCols');
    this.dimensions=new Sequence<civita__NamedDimension>(this.$prefix()+'.dimensions',civita__NamedDimension);
  }
  async counter(...args: boolean[]): Promise<boolean> {return this.$callMethod('counter',...args);}
  async dataRowOffset(...args: number[]): Promise<number> {return this.$callMethod('dataRowOffset',...args);}
  async decSeparator(...args: number[]): Promise<number> {return this.$callMethod('decSeparator',...args);}
  async dontFail(...args: boolean[]): Promise<boolean> {return this.$callMethod('dontFail',...args);}
  async escape(...args: number[]): Promise<number> {return this.$callMethod('escape',...args);}
  async headerRow(...args: number[]): Promise<number> {return this.$callMethod('headerRow',...args);}
  async mergeDelimiters(...args: boolean[]): Promise<boolean> {return this.$callMethod('mergeDelimiters',...args);}
  async quote(...args: number[]): Promise<number> {return this.$callMethod('quote',...args);}
  async separator(...args: number[]): Promise<number> {return this.$callMethod('separator',...args);}
}

export class ravel__HandleState extends CppClass {
  customOrder: Sequence<string>;
  customOrderComplement: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.customOrder=new Sequence<string>(this.$prefix()+'.customOrder');
    this.customOrderComplement=new Sequence<string>(this.$prefix()+'.customOrderComplement');
  }
  async collapsed(...args: boolean[]): Promise<boolean> {return this.$callMethod('collapsed',...args);}
  async customOrderIsInverted(...args: boolean[]): Promise<boolean> {return this.$callMethod('customOrderIsInverted',...args);}
  async description(...args: string[]): Promise<string> {return this.$callMethod('description',...args);}
  async displayFilterCaliper(...args: boolean[]): Promise<boolean> {return this.$callMethod('displayFilterCaliper',...args);}
  async format(...args: string[]): Promise<string> {return this.$callMethod('format',...args);}
  async maxLabel(...args: string[]): Promise<string> {return this.$callMethod('maxLabel',...args);}
  async minLabel(...args: string[]): Promise<string> {return this.$callMethod('minLabel',...args);}
  async order(...args: string[]): Promise<string> {return this.$callMethod('order',...args);}
  async reductionOp(...args: string[]): Promise<string> {return this.$callMethod('reductionOp',...args);}
  async sliceLabel(...args: string[]): Promise<string> {return this.$callMethod('sliceLabel',...args);}
  async x(...args: number[]): Promise<number> {return this.$callMethod('x',...args);}
  async y(...args: number[]): Promise<number> {return this.$callMethod('y',...args);}
}

export class ravel__RavelState extends CppClass {
  handleStates: Sequence<ravel__HandleState>;
  outputHandles: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.handleStates=new Sequence<ravel__HandleState>(this.$prefix()+'.handleStates',ravel__HandleState);
    this.outputHandles=new Sequence<string>(this.$prefix()+'.outputHandles');
  }
  async clear(): Promise<void> {return this.$callMethod('clear');}
  async empty(): Promise<boolean> {return this.$callMethod('empty');}
  async radius(...args: number[]): Promise<number> {return this.$callMethod('radius',...args);}
}

export var minsky=new Minsky('minsky');
