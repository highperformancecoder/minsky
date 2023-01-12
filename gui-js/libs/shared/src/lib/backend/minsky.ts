/*
This is a built file, please do not edit.
See RESTService/typescriptAPI for more information.
*/

import {CppClass, Sequence, Container, Map, Pair} from './backend';

class minsky__dummy {}
class classdesc__json_pack_t {}
class classdesc__pack_t {}
class classdesc__TCL_obj_t {}
class ecolab__cairo__Surface {}
class ecolab__Pango {}

export class Item extends CppClass {
  bb: BoundingBox;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.prefix()+'/bb');
  }
  RESTProcess(a1: minsky__dummy,a2: string): void {return this.callMethod('RESTProcess',a1,a2);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: boolean[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bottom(): number {return this.callMethod('bottom');}
  checkUnits(): object {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): Item {return this.callMethod('clone');}
  closestInPort(a1: number,a2: number): object {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): object {return this.callMethod('closestOutPort',a1,a2);}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  corners(): object[] {return this.callMethod('corners');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: minsky__dummy,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  drawPorts(a1: minsky__dummy): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: minsky__dummy): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: minsky__dummy): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  flip(): void {return this.callMethod('flip');}
  height(): number {return this.callMethod('height');}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  ioVar(): boolean {return this.callMethod('ioVar');}
  itemPtrFromThis(): object {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: boolean[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  left(): number {return this.callMethod('left');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  mouseFocus(...args: boolean[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  onBorder(...args: boolean[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: boolean[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): object {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  right(): number {return this.callMethod('right');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): object {return this.callMethod('select',a1,a2);}
  selected(...args: boolean[]): boolean {return this.callMethod('selected',...args);}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  value(): number {return this.callMethod('value');}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
}

export class OperationBase extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
  }
  addPorts(): void {return this.callMethod('addPorts');}
  arg(...args: number[]): number {return this.callMethod('arg',...args);}
  axis(...args: string[]): string {return this.callMethod('axis',...args);}
  classify(a1: string): string {return this.callMethod('classify',a1);}
  create(a1: string): OperationBase {return this.callMethod('create',a1);}
  dimensions(): string[] {return this.callMethod('dimensions');}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  drawResizeHandles(a1: minsky__dummy): void {return this.callMethod('drawResizeHandles',a1);}
  h(...args: number[]): number {return this.callMethod('h',...args);}
  iconDraw(a1: minsky__dummy): void {return this.callMethod('iconDraw',a1);}
  l(...args: number[]): number {return this.callMethod('l',...args);}
  multiWire(): boolean {return this.callMethod('multiWire');}
  numPorts(): number {return this.callMethod('numPorts');}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  operationCast(): OperationBase {return this.callMethod('operationCast');}
  portValues(): string {return this.callMethod('portValues');}
  r(...args: number[]): number {return this.callMethod('r',...args);}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeHandleCoords(): object {return this.callMethod('resizeHandleCoords');}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  value(): number {return this.callMethod('value');}
}

export class VariableBase extends Item {
  bb: BoundingBox;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.bb=new BoundingBox(this.prefix()+'/bb');
  }
  RESTProcess(a1: minsky__dummy,a2: string): void {return this.callMethod('RESTProcess',a1,a2);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  accessibleVars(): string[] {return this.callMethod('accessibleVars');}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustSliderBounds(): void {return this.callMethod('adjustSliderBounds');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: boolean[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bottom(): number {return this.callMethod('bottom');}
  checkUnits(): object {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): Item {return this.callMethod('clone');}
  closestInPort(a1: number,a2: number): object {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): object {return this.callMethod('closestOutPort',a1,a2);}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  corners(): object[] {return this.callMethod('corners');}
  create(a1: string): VariableBase {return this.callMethod('create',a1);}
  defined(): boolean {return this.callMethod('defined');}
  definition(): string {return this.callMethod('definition');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  dimLabels(): string[] {return this.callMethod('dimLabels');}
  dims(): number[] {return this.callMethod('dims');}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: minsky__dummy,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  drawPorts(a1: minsky__dummy): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: minsky__dummy): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: minsky__dummy): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  engExp(): object {return this.callMethod('engExp');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  ensureValueExists(a1: VariableValue,a2: string): void {return this.callMethod('ensureValueExists',a1,a2);}
  exportAsCSV(a1: string): void {return this.callMethod('exportAsCSV',a1);}
  flip(): void {return this.callMethod('flip');}
  getDimLabelsPicked(): object {return this.callMethod('getDimLabelsPicked');}
  height(): number {return this.callMethod('height');}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  importFromCSV(a1: string,a2: DataSpecSchema): void {return this.callMethod('importFromCSV',a1,a2);}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  init(...args: any[]): string {return this.callMethod('init',...args);}
  initSliderBounds(): void {return this.callMethod('initSliderBounds');}
  inputWired(): boolean {return this.callMethod('inputWired');}
  ioVar(): boolean {return this.callMethod('ioVar');}
  isStock(): boolean {return this.callMethod('isStock');}
  itemPtrFromThis(): object {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: boolean[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  left(): number {return this.callMethod('left');}
  lhs(): boolean {return this.callMethod('lhs');}
  local(): boolean {return this.callMethod('local');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  makeConsistentWithValue(): void {return this.callMethod('makeConsistentWithValue');}
  mantissa(a1: EngNotation,a2: number): string {return this.callMethod('mantissa',a1,a2);}
  maxSliderSteps(): number {return this.callMethod('maxSliderSteps');}
  mouseFocus(...args: boolean[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  name(...args: any[]): string {return this.callMethod('name',...args);}
  numPorts(): number {return this.callMethod('numPorts');}
  onBorder(...args: boolean[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: boolean[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): object {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  rawName(): string {return this.callMethod('rawName');}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeHandleCoords(): object {return this.callMethod('resizeHandleCoords');}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  retype(a1: string): void {return this.callMethod('retype',a1);}
  right(): number {return this.callMethod('right');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): object {return this.callMethod('select',a1,a2);}
  selected(...args: boolean[]): boolean {return this.callMethod('selected',...args);}
  setDimLabelsPicked(a1: string,a2: string): object {return this.callMethod('setDimLabelsPicked',a1,a2);}
  setUnits(a1: string): void {return this.callMethod('setUnits',a1);}
  sliderBoundsSet(...args: boolean[]): boolean {return this.callMethod('sliderBoundsSet',...args);}
  sliderMax(...args: number[]): number {return this.callMethod('sliderMax',...args);}
  sliderMin(...args: number[]): number {return this.callMethod('sliderMin',...args);}
  sliderSet(a1: number): void {return this.callMethod('sliderSet',a1);}
  sliderStep(...args: number[]): number {return this.callMethod('sliderStep',...args);}
  sliderStepRel(...args: boolean[]): boolean {return this.callMethod('sliderStepRel',...args);}
  temp(): boolean {return this.callMethod('temp');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  toggleLocal(): void {return this.callMethod('toggleLocal');}
  toggleVarTabDisplay(): void {return this.callMethod('toggleVarTabDisplay');}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  units(...args: any[]): object {return this.callMethod('units',...args);}
  unitsStr(): string {return this.callMethod('unitsStr');}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  vValue(): object {return this.callMethod('vValue');}
  value(...args: any[]): number {return this.callMethod('value',...args);}
  valueId(): string {return this.callMethod('valueId');}
  valueIdInCurrentScope(a1: string): string {return this.callMethod('valueIdInCurrentScope',a1);}
  varTabDisplay(...args: boolean[]): boolean {return this.callMethod('varTabDisplay',...args);}
  variableCast(): VariableBase {return this.callMethod('variableCast');}
  varsPassed(...args: number[]): number {return this.callMethod('varsPassed',...args);}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
}

export class Bookmark extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  name(...args: string[]): string {return this.callMethod('name',...args);}
  x(...args: number[]): number {return this.callMethod('x',...args);}
  y(...args: number[]): number {return this.callMethod('y',...args);}
  zoom(...args: number[]): number {return this.callMethod('zoom',...args);}
}

export class BoundingBox extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  bottom(): number {return this.callMethod('bottom');}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  height(): number {return this.callMethod('height');}
  left(): number {return this.callMethod('left');}
  right(): number {return this.callMethod('right');}
  top(): number {return this.callMethod('top');}
  update(a1: Item): void {return this.callMethod('update',a1);}
  valid(): boolean {return this.callMethod('valid');}
  width(): number {return this.callMethod('width');}
}

export class CSVDialog extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  spec: DataSpec;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.spec=new DataSpec(this.prefix()+'/spec');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  colWidth(...args: number[]): number {return this.callMethod('colWidth',...args);}
  columnOver(a1: number): number {return this.callMethod('columnOver',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  copyHeaderRowToDimNames(a1: number): void {return this.callMethod('copyHeaderRowToDimNames',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  flashNameRow(...args: boolean[]): boolean {return this.callMethod('flashNameRow',...args);}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  guessSpecAndLoadFile(): void {return this.callMethod('guessSpecAndLoadFile');}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  headerForCol(a1: number): string {return this.callMethod('headerForCol',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  loadFile(): void {return this.callMethod('loadFile');}
  loadFileFromName(a1: string): void {return this.callMethod('loadFileFromName',a1);}
  loadWebFile(a1: string): string {return this.callMethod('loadWebFile',a1);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numInitialLines(...args: number[]): number {return this.callMethod('numInitialLines',...args);}
  parseLines(): string[][] {return this.callMethod('parseLines');}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  reportFromFile(a1: string,a2: string): void {return this.callMethod('reportFromFile',a1,a2);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowOver(a1: number): number {return this.callMethod('rowOver',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  tableWidth(): number {return this.callMethod('tableWidth');}
  url(...args: string[]): string {return this.callMethod('url',...args);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class Canvas extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  itemFocus: Item;
  itemVector: Sequence<Item>;
  lasso: LassoBox;
  model: Group;
  selection: Selection;
  updateRegion: LassoBox;
  wire: Wire;
  wireFocus: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.itemFocus=new Item(this.prefix()+'/itemFocus');
    this.itemVector=new Sequence<Item>(this.prefix()+'/itemVector',Item);
    this.lasso=new LassoBox(this.prefix()+'/lasso');
    this.model=new Group(this.prefix()+'/model');
    this.selection=new Selection(this.prefix()+'/selection');
    this.updateRegion=new LassoBox(this.prefix()+'/updateRegion');
    this.wire=new Wire(this.prefix()+'/wire');
    this.wireFocus=new Wire(this.prefix()+'/wireFocus');
  }
  addGodley(): void {return this.callMethod('addGodley');}
  addGroup(): void {return this.callMethod('addGroup');}
  addLock(): void {return this.callMethod('addLock');}
  addNote(a1: string): void {return this.callMethod('addNote',a1);}
  addOperation(a1: string): void {return this.callMethod('addOperation',a1);}
  addPlot(): void {return this.callMethod('addPlot');}
  addRavel(): void {return this.callMethod('addRavel');}
  addSheet(): void {return this.callMethod('addSheet');}
  addSwitch(): void {return this.callMethod('addSwitch');}
  addVariable(a1: string,a2: string): void {return this.callMethod('addVariable',a1,a2);}
  clickType(...args: string[]): string {return this.callMethod('clickType',...args);}
  closestInPort(a1: number,a2: number): object {return this.callMethod('closestInPort',a1,a2);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  copyAllFlowVars(): void {return this.callMethod('copyAllFlowVars');}
  copyAllStockVars(): void {return this.callMethod('copyAllStockVars');}
  copyItem(): void {return this.callMethod('copyItem');}
  defaultRotation(...args: number[]): number {return this.callMethod('defaultRotation',...args);}
  delHandle(a1: number,a2: number): void {return this.callMethod('delHandle',a1,a2);}
  deleteItem(): void {return this.callMethod('deleteItem');}
  deleteWire(): void {return this.callMethod('deleteWire');}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(): void {return this.callMethod('draw');}
  findVariableDefinition(): boolean {return this.callMethod('findVariableDefinition');}
  focusFollowsMouse(...args: boolean[]): boolean {return this.callMethod('focusFollowsMouse',...args);}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  groupSelection(): void {return this.callMethod('groupSelection');}
  handleSelected(...args: number[]): number {return this.callMethod('handleSelected',...args);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  itemAt(a1: number,a2: number): object {return this.callMethod('itemAt',a1,a2);}
  itemIndicator(...args: boolean[]): boolean {return this.callMethod('itemIndicator',...args);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  lassoMode(...args: string[]): string {return this.callMethod('lassoMode',...args);}
  lockRavelsInSelection(): void {return this.callMethod('lockRavelsInSelection');}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  openGroupInCanvas(a1: Item): void {return this.callMethod('openGroupInCanvas',a1);}
  position(): number[] {return this.callMethod('position');}
  pushDefiningVarsToTab(): void {return this.callMethod('pushDefiningVarsToTab');}
  ravelsSelected(): number {return this.callMethod('ravelsSelected');}
  recentre(): void {return this.callMethod('recentre');}
  redraw(...args: number[]): boolean {return this.callMethod('redraw',...args);}
  redrawAll(...args: boolean[]): boolean {return this.callMethod('redrawAll',...args);}
  redrawRequested(): boolean {return this.callMethod('redrawRequested');}
  redrawUpdateRegion(): boolean {return this.callMethod('redrawUpdateRegion');}
  registerImage(): void {return this.callMethod('registerImage');}
  removeItemFromItsGroup(): void {return this.callMethod('removeItemFromItsGroup');}
  renameAllInstances(a1: string): void {return this.callMethod('renameAllInstances',a1);}
  renameItem(a1: string): void {return this.callMethod('renameItem',a1);}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  select(...args: any[]): void {return this.callMethod('select',...args);}
  selectAllVariables(): void {return this.callMethod('selectAllVariables');}
  selectVar(a1: number,a2: number): boolean {return this.callMethod('selectVar',a1,a2);}
  setItemFocus(a1: Item): void {return this.callMethod('setItemFocus',a1);}
  showDefiningVarsOnCanvas(): void {return this.callMethod('showDefiningVarsOnCanvas');}
  showPlotsOnTab(): void {return this.callMethod('showPlotsOnTab');}
  surface(): ecolab__cairo__Surface {return this.callMethod('surface');}
  termX(...args: number[]): number {return this.callMethod('termX',...args);}
  termY(...args: number[]): number {return this.callMethod('termY',...args);}
  ungroupItem(): void {return this.callMethod('ungroupItem');}
  unlockRavelsInSelection(): void {return this.callMethod('unlockRavelsInSelection');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
  zoomToDisplay(): void {return this.callMethod('zoomToDisplay');}
}

export class DataOp extends Item {
  data: Map<number,number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.data=new Map<number,number>(this.prefix()+'/data');
  }
  deriv(a1: number): number {return this.callMethod('deriv',a1);}
  description(...args: any[]): string {return this.callMethod('description',...args);}
  initRandom(a1: number,a2: number,a3: number): void {return this.callMethod('initRandom',a1,a2,a3);}
  interpolate(a1: number): number {return this.callMethod('interpolate',a1);}
  pack(a1: classdesc__pack_t,a2: string): void {return this.callMethod('pack',a1,a2);}
  readData(a1: string): void {return this.callMethod('readData',a1);}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  unpack(a1: classdesc__pack_t,a2: string): void {return this.callMethod('unpack',a1,a2);}
}

export class DataSpec extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensionNames: Sequence<string>;
  dimensions: Sequence<civita__Dimension>;
  horizontalDimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(this.prefix()+'/dataCols');
    this.dimensionCols=new Container<number>(this.prefix()+'/dimensionCols');
    this.dimensionNames=new Sequence<string>(this.prefix()+'/dimensionNames');
    this.dimensions=new Sequence<civita__Dimension>(this.prefix()+'/dimensions',civita__Dimension);
    this.horizontalDimension=new civita__Dimension(this.prefix()+'/horizontalDimension');
  }
  columnar(...args: boolean[]): boolean {return this.callMethod('columnar',...args);}
  dataColOffset(...args: number[]): number {return this.callMethod('dataColOffset',...args);}
  dataRowOffset(...args: number[]): number {return this.callMethod('dataRowOffset',...args);}
  decSeparator(...args: number[]): number {return this.callMethod('decSeparator',...args);}
  duplicateKeyAction(...args: string[]): string {return this.callMethod('duplicateKeyAction',...args);}
  escape(...args: number[]): number {return this.callMethod('escape',...args);}
  guessFromFile(a1: string): void {return this.callMethod('guessFromFile',a1);}
  guessFromStream(a1: minsky__dummy): void {return this.callMethod('guessFromStream',a1);}
  headerRow(...args: number[]): number {return this.callMethod('headerRow',...args);}
  horizontalDimName(...args: string[]): string {return this.callMethod('horizontalDimName',...args);}
  mergeDelimiters(...args: boolean[]): boolean {return this.callMethod('mergeDelimiters',...args);}
  missingValue(...args: number[]): number {return this.callMethod('missingValue',...args);}
  nColAxes(): number {return this.callMethod('nColAxes');}
  nRowAxes(): number {return this.callMethod('nRowAxes');}
  populateFromRavelMetadata(a1: string,a2: number): void {return this.callMethod('populateFromRavelMetadata',a1,a2);}
  quote(...args: number[]): number {return this.callMethod('quote',...args);}
  separator(...args: number[]): number {return this.callMethod('separator',...args);}
  setDataArea(a1: number,a2: number): void {return this.callMethod('setDataArea',a1,a2);}
  toSchema(): object {return this.callMethod('toSchema');}
  toggleDimension(a1: number): void {return this.callMethod('toggleDimension',a1);}
}

export class DataSpecSchema extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensionNames: Sequence<string>;
  dimensions: Sequence<civita__Dimension>;
  horizontalDimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(this.prefix()+'/dataCols');
    this.dimensionCols=new Container<number>(this.prefix()+'/dimensionCols');
    this.dimensionNames=new Sequence<string>(this.prefix()+'/dimensionNames');
    this.dimensions=new Sequence<civita__Dimension>(this.prefix()+'/dimensions',civita__Dimension);
    this.horizontalDimension=new civita__Dimension(this.prefix()+'/horizontalDimension');
  }
  columnar(...args: boolean[]): boolean {return this.callMethod('columnar',...args);}
  dataColOffset(...args: number[]): number {return this.callMethod('dataColOffset',...args);}
  dataRowOffset(...args: number[]): number {return this.callMethod('dataRowOffset',...args);}
  decSeparator(...args: number[]): number {return this.callMethod('decSeparator',...args);}
  duplicateKeyAction(...args: string[]): string {return this.callMethod('duplicateKeyAction',...args);}
  escape(...args: number[]): number {return this.callMethod('escape',...args);}
  headerRow(...args: number[]): number {return this.callMethod('headerRow',...args);}
  horizontalDimName(...args: string[]): string {return this.callMethod('horizontalDimName',...args);}
  mergeDelimiters(...args: boolean[]): boolean {return this.callMethod('mergeDelimiters',...args);}
  missingValue(...args: number[]): number {return this.callMethod('missingValue',...args);}
  quote(...args: number[]): number {return this.callMethod('quote',...args);}
  separator(...args: number[]): number {return this.callMethod('separator',...args);}
}

export class EngNotation extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  engExp(...args: number[]): number {return this.callMethod('engExp',...args);}
  sciExp(...args: number[]): number {return this.callMethod('sciExp',...args);}
}

export class EquationDisplay extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  position(): number[] {return this.callMethod('position');}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class EvalGodley extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  compatibility(...args: boolean[]): boolean {return this.callMethod('compatibility',...args);}
  eval(a1: number,a2: number): void {return this.callMethod('eval',a1,a2);}
}

export class FontDisplay extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
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
      super(prefix.prefix())
    this.editor=new GodleyTableEditor(this.prefix()+'/editor');
    this.popup=new GodleyTableWindow(this.prefix()+'/popup');
    this.svgRenderer=new SVGRenderer(this.prefix()+'/svgRenderer');
    this.table=new GodleyTable(this.prefix()+'/table');
  }
  adjustPopupWidgets(): void {return this.callMethod('adjustPopupWidgets');}
  bottomMargin(): number {return this.callMethod('bottomMargin');}
  buttonDisplay(): boolean {return this.callMethod('buttonDisplay');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): GodleyIcon {return this.callMethod('clone');}
  deleteRow(a1: number): void {return this.callMethod('deleteRow',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  editorMode(): boolean {return this.callMethod('editorMode');}
  flowSignature(a1: number): object {return this.callMethod('flowSignature',a1);}
  flowVars(): Sequence<VariableBase> {return this.callMethod('flowVars');}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  insertControlled(a1: Selection): void {return this.callMethod('insertControlled',a1);}
  leftMargin(): number {return this.callMethod('leftMargin');}
  moveCell(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('moveCell',a1,a2,a3,a4);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  removeControlledItems(a1: GroupItems): void {return this.callMethod('removeControlledItems',a1);}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  rowSum(a1: number): string {return this.callMethod('rowSum',a1);}
  scaleIcon(a1: number,a2: number): void {return this.callMethod('scaleIcon',a1,a2);}
  select(a1: number,a2: number): object {return this.callMethod('select',a1,a2);}
  setCell(a1: number,a2: number,a3: string): void {return this.callMethod('setCell',a1,a2,a3);}
  setCurrency(a1: string): void {return this.callMethod('setCurrency',a1);}
  stockVarUnits(a1: string,a2: boolean): object {return this.callMethod('stockVarUnits',a1,a2);}
  stockVars(): Sequence<VariableBase> {return this.callMethod('stockVars');}
  toggleButtons(): void {return this.callMethod('toggleButtons');}
  toggleEditorMode(): void {return this.callMethod('toggleEditorMode');}
  toggleVariableDisplay(): void {return this.callMethod('toggleVariableDisplay');}
  update(): void {return this.callMethod('update');}
  valueId(a1: string): string {return this.callMethod('valueId',a1);}
  variableDisplay(...args: boolean[]): boolean {return this.callMethod('variableDisplay',...args);}
}

export class GodleyTab extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  bottomRowMargin: Sequence<number>;
  item: Item;
  itemFocus: Item;
  itemVector: Sequence<Item>;
  rightColMargin: Sequence<number>;
  varAttrib: Sequence<string>;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.prefix()+'/bottomRowMargin');
    this.item=new Item(this.prefix()+'/item');
    this.itemFocus=new Item(this.prefix()+'/itemFocus');
    this.itemVector=new Sequence<Item>(this.prefix()+'/itemVector',Item);
    this.rightColMargin=new Sequence<number>(this.prefix()+'/rightColMargin');
    this.varAttrib=new Sequence<string>(this.prefix()+'/varAttrib');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cell(a1: number,a2: number): ecolab__Pango {return this.callMethod('cell',a1,a2);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): object {return this.callMethod('itemAt',a1,a2);}
  itemSelector(a1: Item): boolean {return this.callMethod('itemSelector',a1);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(): number {return this.callMethod('numCols');}
  numRows(): number {return this.callMethod('numRows');}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xItem(...args: number[]): number {return this.callMethod('xItem',...args);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  yItem(...args: number[]): number {return this.callMethod('yItem',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class GodleyTable extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  _assetClass(...args: any[]): Sequence<string> {return this.callMethod('_assetClass',...args);}
  assetClass(a1: object): string {return this.callMethod('assetClass',a1);}
  cell(a1: number,a2: number): string {return this.callMethod('cell',a1,a2);}
  cellInTable(a1: number,a2: number): boolean {return this.callMethod('cellInTable',a1,a2);}
  clear(): void {return this.callMethod('clear');}
  cols(): number {return this.callMethod('cols');}
  deleteCol(a1: number): void {return this.callMethod('deleteCol',a1);}
  deleteRow(a1: number): void {return this.callMethod('deleteRow',a1);}
  dimension(a1: number,a2: number): void {return this.callMethod('dimension',a1,a2);}
  doubleEntryCompliant(...args: boolean[]): boolean {return this.callMethod('doubleEntryCompliant',...args);}
  exportToCSV(a1: string): void {return this.callMethod('exportToCSV',a1);}
  exportToLaTeX(a1: string): void {return this.callMethod('exportToLaTeX',a1);}
  getCell(a1: number,a2: number): string {return this.callMethod('getCell',a1,a2);}
  getColumn(a1: number): string[] {return this.callMethod('getColumn',a1);}
  getColumnVariables(): string[] {return this.callMethod('getColumnVariables');}
  getData(): Sequence<string[]> {return this.callMethod('getData');}
  getVariables(): string[] {return this.callMethod('getVariables');}
  initialConditionRow(a1: number): boolean {return this.callMethod('initialConditionRow',a1);}
  initialConditions(...args: string[]): string {return this.callMethod('initialConditions',...args);}
  insertCol(a1: number): void {return this.callMethod('insertCol',a1);}
  insertRow(a1: number): void {return this.callMethod('insertRow',a1);}
  moveCol(a1: number,a2: number): void {return this.callMethod('moveCol',a1,a2);}
  moveRow(a1: number,a2: number): void {return this.callMethod('moveRow',a1,a2);}
  nameUnique(): void {return this.callMethod('nameUnique');}
  orderAssetClasses(): void {return this.callMethod('orderAssetClasses');}
  rename(a1: string,a2: string): void {return this.callMethod('rename',a1,a2);}
  renameFlows(a1: string,a2: string): void {return this.callMethod('renameFlows',a1,a2);}
  renameStock(a1: string,a2: string): void {return this.callMethod('renameStock',a1,a2);}
  resize(a1: number,a2: number): void {return this.callMethod('resize',a1,a2);}
  rowSum(a1: number): string {return this.callMethod('rowSum',a1);}
  rows(): number {return this.callMethod('rows');}
  savedText(...args: string[]): string {return this.callMethod('savedText',...args);}
  setDEmode(a1: boolean): void {return this.callMethod('setDEmode',a1);}
  signConventionReversed(a1: number): boolean {return this.callMethod('signConventionReversed',a1);}
  singleEquity(): boolean {return this.callMethod('singleEquity');}
  singularRow(a1: number,a2: number): boolean {return this.callMethod('singularRow',a1,a2);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
}

export class GodleyTableEditor extends CppClass {
  colLeftMargin: Sequence<number>;
  constructor(prefix: string){
    super(prefix);
    this.colLeftMargin=new Sequence<number>(this.prefix()+'/colLeftMargin');
  }
  addFlow(a1: number): void {return this.callMethod('addFlow',a1);}
  addStockVar(a1: number): void {return this.callMethod('addStockVar',a1);}
  adjustWidgets(): void {return this.callMethod('adjustWidgets');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clickTypeZoomed(a1: number,a2: number): string {return this.callMethod('clickTypeZoomed',a1,a2);}
  colXZoomed(a1: number): number {return this.callMethod('colXZoomed',a1);}
  columnButtonsOffset(...args: number[]): number {return this.callMethod('columnButtonsOffset',...args);}
  copy(): void {return this.callMethod('copy');}
  cut(): void {return this.callMethod('cut');}
  delSelection(): void {return this.callMethod('delSelection');}
  deleteFlow(a1: number): void {return this.callMethod('deleteFlow',a1);}
  deleteStockVar(a1: number): void {return this.callMethod('deleteStockVar',a1);}
  disableButtons(): void {return this.callMethod('disableButtons');}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  drawButtons(...args: boolean[]): boolean {return this.callMethod('drawButtons',...args);}
  enableButtons(): void {return this.callMethod('enableButtons');}
  godleyIcon(): GodleyIcon {return this.callMethod('godleyIcon');}
  height(): number {return this.callMethod('height');}
  highlightCell(a1: minsky__dummy,a2: number,a3: number): void {return this.callMethod('highlightCell',a1,a2,a3);}
  highlightColumn(a1: minsky__dummy,a2: number): void {return this.callMethod('highlightColumn',a1,a2);}
  highlightRow(a1: minsky__dummy,a2: number): void {return this.callMethod('highlightRow',a1,a2);}
  historyPtr(...args: number[]): number {return this.callMethod('historyPtr',...args);}
  hoverCol(...args: number[]): number {return this.callMethod('hoverCol',...args);}
  hoverRow(...args: number[]): number {return this.callMethod('hoverRow',...args);}
  importStockVar(a1: string,a2: number): void {return this.callMethod('importStockVar',a1,a2);}
  insertIdx(...args: number[]): number {return this.callMethod('insertIdx',...args);}
  keyPress(a1: number,a2: string): void {return this.callMethod('keyPress',a1,a2);}
  leftTableOffset(...args: number[]): number {return this.callMethod('leftTableOffset',...args);}
  matchingTableColumns(a1: number): string[] {return this.callMethod('matchingTableColumns',a1);}
  maxHistory(...args: number[]): number {return this.callMethod('maxHistory',...args);}
  minColumnWidth(...args: number[]): number {return this.callMethod('minColumnWidth',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseMoveB1(a1: number,a2: number): void {return this.callMethod('mouseMoveB1',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveAssetClass(a1: number,a2: number): string {return this.callMethod('moveAssetClass',a1,a2);}
  navigateDown(): void {return this.callMethod('navigateDown');}
  navigateLeft(): void {return this.callMethod('navigateLeft');}
  navigateRight(): void {return this.callMethod('navigateRight');}
  navigateUp(): void {return this.callMethod('navigateUp');}
  paste(): void {return this.callMethod('paste');}
  pulldownHot(...args: number[]): number {return this.callMethod('pulldownHot',...args);}
  pushHistory(): void {return this.callMethod('pushHistory');}
  rowHeight(...args: number[]): number {return this.callMethod('rowHeight',...args);}
  rowYZoomed(a1: number): number {return this.callMethod('rowYZoomed',a1);}
  scrollColStart(...args: number[]): number {return this.callMethod('scrollColStart',...args);}
  scrollRowStart(...args: number[]): number {return this.callMethod('scrollRowStart',...args);}
  selectIdx(...args: number[]): number {return this.callMethod('selectIdx',...args);}
  selectedCellInTable(): boolean {return this.callMethod('selectedCellInTable');}
  selectedCol(...args: number[]): number {return this.callMethod('selectedCol',...args);}
  selectedRow(...args: number[]): number {return this.callMethod('selectedRow',...args);}
  srcCol(...args: number[]): number {return this.callMethod('srcCol',...args);}
  srcRow(...args: number[]): number {return this.callMethod('srcRow',...args);}
  swapAssetClass(a1: number,a2: number): string {return this.callMethod('swapAssetClass',a1,a2);}
  textIdx(a1: number): number {return this.callMethod('textIdx',a1);}
  topTableOffset(...args: number[]): number {return this.callMethod('topTableOffset',...args);}
  undo(a1: number): void {return this.callMethod('undo',a1);}
  update(): void {return this.callMethod('update');}
  width(): number {return this.callMethod('width');}
  zoomFactor(...args: number[]): number {return this.callMethod('zoomFactor',...args);}
}

export class GodleyTableWindow extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  colLeftMargin: Sequence<number>;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.colLeftMargin=new Sequence<number>(this.prefix()+'/colLeftMargin');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  addFlow(a1: number): void {return this.callMethod('addFlow',a1);}
  addStockVar(a1: number): void {return this.callMethod('addStockVar',a1);}
  adjustWidgets(): void {return this.callMethod('adjustWidgets');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clickTypeZoomed(a1: number,a2: number): string {return this.callMethod('clickTypeZoomed',a1,a2);}
  colXZoomed(a1: number): number {return this.callMethod('colXZoomed',a1);}
  columnButtonsOffset(...args: number[]): number {return this.callMethod('columnButtonsOffset',...args);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  copy(): void {return this.callMethod('copy');}
  cut(): void {return this.callMethod('cut');}
  delSelection(): void {return this.callMethod('delSelection');}
  deleteFlow(a1: number): void {return this.callMethod('deleteFlow',a1);}
  deleteStockVar(a1: number): void {return this.callMethod('deleteStockVar',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  disableButtons(): void {return this.callMethod('disableButtons');}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  drawButtons(...args: boolean[]): boolean {return this.callMethod('drawButtons',...args);}
  enableButtons(): void {return this.callMethod('enableButtons');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  godleyIcon(): GodleyIcon {return this.callMethod('godleyIcon');}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  highlightCell(a1: minsky__dummy,a2: number,a3: number): void {return this.callMethod('highlightCell',a1,a2,a3);}
  highlightColumn(a1: minsky__dummy,a2: number): void {return this.callMethod('highlightColumn',a1,a2);}
  highlightRow(a1: minsky__dummy,a2: number): void {return this.callMethod('highlightRow',a1,a2);}
  historyPtr(...args: number[]): number {return this.callMethod('historyPtr',...args);}
  hoverCol(...args: number[]): number {return this.callMethod('hoverCol',...args);}
  hoverRow(...args: number[]): number {return this.callMethod('hoverRow',...args);}
  importStockVar(a1: string,a2: number): void {return this.callMethod('importStockVar',a1,a2);}
  insertIdx(...args: number[]): number {return this.callMethod('insertIdx',...args);}
  keyPress(...args: any[]): boolean {return this.callMethod('keyPress',...args);}
  leftTableOffset(...args: number[]): number {return this.callMethod('leftTableOffset',...args);}
  matchingTableColumns(a1: number): string[] {return this.callMethod('matchingTableColumns',a1);}
  maxHistory(...args: number[]): number {return this.callMethod('maxHistory',...args);}
  minColumnWidth(...args: number[]): number {return this.callMethod('minColumnWidth',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseMoveB1(a1: number,a2: number): void {return this.callMethod('mouseMoveB1',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveAssetClass(a1: number,a2: number): string {return this.callMethod('moveAssetClass',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  navigateDown(): void {return this.callMethod('navigateDown');}
  navigateLeft(): void {return this.callMethod('navigateLeft');}
  navigateRight(): void {return this.callMethod('navigateRight');}
  navigateUp(): void {return this.callMethod('navigateUp');}
  paste(): void {return this.callMethod('paste');}
  position(): number[] {return this.callMethod('position');}
  pulldownHot(...args: number[]): number {return this.callMethod('pulldownHot',...args);}
  pushHistory(): void {return this.callMethod('pushHistory');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  requestRedrawCanvas(): void {return this.callMethod('requestRedrawCanvas');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowHeight(...args: number[]): number {return this.callMethod('rowHeight',...args);}
  rowYZoomed(a1: number): number {return this.callMethod('rowYZoomed',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  scrollColStart(...args: number[]): number {return this.callMethod('scrollColStart',...args);}
  scrollRowStart(...args: number[]): number {return this.callMethod('scrollRowStart',...args);}
  selectIdx(...args: number[]): number {return this.callMethod('selectIdx',...args);}
  selectedCellInTable(): boolean {return this.callMethod('selectedCellInTable');}
  selectedCol(...args: number[]): number {return this.callMethod('selectedCol',...args);}
  selectedRow(...args: number[]): number {return this.callMethod('selectedRow',...args);}
  srcCol(...args: number[]): number {return this.callMethod('srcCol',...args);}
  srcRow(...args: number[]): number {return this.callMethod('srcRow',...args);}
  swapAssetClass(a1: number,a2: number): string {return this.callMethod('swapAssetClass',a1,a2);}
  textIdx(a1: number): number {return this.callMethod('textIdx',a1);}
  topTableOffset(...args: number[]): number {return this.callMethod('topTableOffset',...args);}
  undo(a1: number): void {return this.callMethod('undo',a1);}
  update(): void {return this.callMethod('update');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
  zoomFactor(...args: number[]): number {return this.callMethod('zoomFactor',...args);}
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
      super(prefix.prefix())
    this.bb=new BoundingBox(this.prefix()+'/bb');
    this.bookmarks=new Container<Bookmark>(this.prefix()+'/bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.prefix()+'/createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.prefix()+'/groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.prefix()+'/inVariables',VariableBase);
    this.items=new Sequence<Item>(this.prefix()+'/items',Item);
    this.outVariables=new Sequence<VariableBase>(this.prefix()+'/outVariables',VariableBase);
    this.svgRenderer=new SVGRenderer(this.prefix()+'/svgRenderer');
    this.wires=new Sequence<Wire>(this.prefix()+'/wires',Wire);
  }
  RESTProcess(a1: minsky__dummy,a2: string): void {return this.callMethod('RESTProcess',a1,a2);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  accessibleVars(): string[] {return this.callMethod('accessibleVars');}
  addBookmark(a1: string): void {return this.callMethod('addBookmark',a1);}
  addBookmarkXY(a1: number,a2: number,a3: string): void {return this.callMethod('addBookmarkXY',a1,a2,a3);}
  addGroup(a1: Group): object {return this.callMethod('addGroup',a1);}
  addInputVar(): void {return this.callMethod('addInputVar');}
  addItem(...args: any[]): object {return this.callMethod('addItem',...args);}
  addOutputVar(): void {return this.callMethod('addOutputVar');}
  addWire(...args: any[]): object {return this.callMethod('addWire',...args);}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustWiresGroup(a1: Wire): void {return this.callMethod('adjustWiresGroup',a1);}
  arguments(): string {return this.callMethod('arguments');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  autoLayout(): void {return this.callMethod('autoLayout');}
  bookmark(...args: boolean[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bookmarkList(): string[] {return this.callMethod('bookmarkList');}
  bottom(): number {return this.callMethod('bottom');}
  cBounds(): number[] {return this.callMethod('cBounds');}
  checkAddIORegion(a1: Item): void {return this.callMethod('checkAddIORegion',a1);}
  checkUnits(): object {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  clear(): void {return this.callMethod('clear');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): Item {return this.callMethod('clone');}
  closestInPort(a1: number,a2: number): object {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): object {return this.callMethod('closestOutPort',a1,a2);}
  computeDisplayZoom(): number {return this.callMethod('computeDisplayZoom');}
  computeRelZoom(): void {return this.callMethod('computeRelZoom');}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  contentBounds(a1: number,a2: number,a3: number,a4: number): number {return this.callMethod('contentBounds',a1,a2,a3,a4);}
  copy(): object {return this.callMethod('copy');}
  copyUnowned(): object {return this.callMethod('copyUnowned');}
  corners(): object[] {return this.callMethod('corners');}
  defaultExtension(): string {return this.callMethod('defaultExtension');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteBookmark(a1: number): void {return this.callMethod('deleteBookmark',a1);}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  deleteItem(a1: Item): void {return this.callMethod('deleteItem',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayContents(): boolean {return this.callMethod('displayContents');}
  displayContentsChanged(): boolean {return this.callMethod('displayContentsChanged');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: minsky__dummy,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  displayZoom(...args: number[]): number {return this.callMethod('displayZoom',...args);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  draw1edge(a1: Sequence<VariableBase>,a2: minsky__dummy,a3: number): void {return this.callMethod('draw1edge',a1,a2,a3);}
  drawEdgeVariables(a1: minsky__dummy): void {return this.callMethod('drawEdgeVariables',a1);}
  drawIORegion(a1: minsky__dummy): void {return this.callMethod('drawIORegion',a1);}
  drawPorts(a1: minsky__dummy): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: minsky__dummy): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: minsky__dummy): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  edgeScale(): number {return this.callMethod('edgeScale');}
  empty(): boolean {return this.callMethod('empty');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  findGroup(a1: Group): object {return this.callMethod('findGroup',a1);}
  findItem(a1: Item): object {return this.callMethod('findItem',a1);}
  findWire(a1: Wire): object {return this.callMethod('findWire',a1);}
  flip(): void {return this.callMethod('flip');}
  flipContents(): void {return this.callMethod('flipContents');}
  formula(): string {return this.callMethod('formula');}
  globalGroup(): Group {return this.callMethod('globalGroup');}
  gotoBookmark(a1: number): void {return this.callMethod('gotoBookmark',a1);}
  gotoBookmark_b(a1: Bookmark): void {return this.callMethod('gotoBookmark_b',a1);}
  height(): number {return this.callMethod('height');}
  higher(a1: Group): boolean {return this.callMethod('higher',a1);}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  inIORegion(a1: number,a2: number): string {return this.callMethod('inIORegion',a1,a2);}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  ioVar(): boolean {return this.callMethod('ioVar');}
  itemPtrFromThis(): object {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: boolean[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  left(): number {return this.callMethod('left');}
  level(): number {return this.callMethod('level');}
  localZoom(): number {return this.callMethod('localZoom');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  makeSubroutine(): void {return this.callMethod('makeSubroutine');}
  margins(a1: number,a2: number): void {return this.callMethod('margins',a1,a2);}
  marginsV(): number[] {return this.callMethod('marginsV');}
  minimalEnclosingGroup(a1: number,a2: number,a3: number,a4: number,a5: Item): Group {return this.callMethod('minimalEnclosingGroup',a1,a2,a3,a4,a5);}
  mouseFocus(...args: boolean[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveContents(a1: Group): void {return this.callMethod('moveContents',a1);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  name(): string {return this.callMethod('name');}
  nocycles(): boolean {return this.callMethod('nocycles');}
  normaliseGroupRefs(a1: Group): void {return this.callMethod('normaliseGroupRefs',a1);}
  numGroups(): number {return this.callMethod('numGroups');}
  numItems(): number {return this.callMethod('numItems');}
  numWires(): number {return this.callMethod('numWires');}
  onBorder(...args: boolean[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: boolean[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): object {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  px(...args: number[]): number {return this.callMethod('px',...args);}
  py(...args: number[]): number {return this.callMethod('py',...args);}
  pz(...args: number[]): number {return this.callMethod('pz',...args);}
  randomLayout(): void {return this.callMethod('randomLayout');}
  relZoom(...args: number[]): number {return this.callMethod('relZoom',...args);}
  removeDisplayPlot(): void {return this.callMethod('removeDisplayPlot');}
  removeGroup(a1: Group): object {return this.callMethod('removeGroup',a1);}
  removeItem(a1: Item): object {return this.callMethod('removeItem',a1);}
  removeWire(a1: Wire): object {return this.callMethod('removeWire',a1);}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  resizeOnContents(): void {return this.callMethod('resizeOnContents');}
  right(): number {return this.callMethod('right');}
  rotFactor(): number {return this.callMethod('rotFactor');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): object {return this.callMethod('select',a1,a2);}
  selected(...args: boolean[]): boolean {return this.callMethod('selected',...args);}
  setZoom(a1: number): void {return this.callMethod('setZoom',a1);}
  splitBoundaryCrossingWires(): void {return this.callMethod('splitBoundaryCrossingWires');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  uniqueItems(...args: any[]): boolean {return this.callMethod('uniqueItems',...args);}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  updateTimestamp(): void {return this.callMethod('updateTimestamp');}
  value(): number {return this.callMethod('value');}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
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
    this.bookmarks=new Container<Bookmark>(this.prefix()+'/bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.prefix()+'/createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.prefix()+'/groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.prefix()+'/inVariables',VariableBase);
    this.items=new Sequence<Item>(this.prefix()+'/items',Item);
    this.outVariables=new Sequence<VariableBase>(this.prefix()+'/outVariables',VariableBase);
    this.wires=new Sequence<Wire>(this.prefix()+'/wires',Wire);
  }
  addGroup(a1: Group): object {return this.callMethod('addGroup',a1);}
  addItem(...args: any[]): object {return this.callMethod('addItem',...args);}
  addWire(...args: any[]): object {return this.callMethod('addWire',...args);}
  adjustWiresGroup(a1: Wire): void {return this.callMethod('adjustWiresGroup',a1);}
  clear(): void {return this.callMethod('clear');}
  empty(): boolean {return this.callMethod('empty');}
  findGroup(a1: Group): object {return this.callMethod('findGroup',a1);}
  findItem(a1: Item): object {return this.callMethod('findItem',a1);}
  findWire(a1: Wire): object {return this.callMethod('findWire',a1);}
  nocycles(): boolean {return this.callMethod('nocycles');}
  numGroups(): number {return this.callMethod('numGroups');}
  numItems(): number {return this.callMethod('numItems');}
  numWires(): number {return this.callMethod('numWires');}
  removeDisplayPlot(): void {return this.callMethod('removeDisplayPlot');}
  removeGroup(a1: Group): object {return this.callMethod('removeGroup',a1);}
  removeItem(a1: Item): object {return this.callMethod('removeItem',a1);}
  removeWire(a1: Wire): object {return this.callMethod('removeWire',a1);}
}

export class HandleLockInfo extends CppClass {
  handleNames: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.handleNames=new Sequence<string>(this.prefix()+'/handleNames');
  }
  calipers(...args: boolean[]): boolean {return this.callMethod('calipers',...args);}
  order(...args: boolean[]): boolean {return this.callMethod('order',...args);}
  orientation(...args: boolean[]): boolean {return this.callMethod('orientation',...args);}
  slicer(...args: boolean[]): boolean {return this.callMethod('slicer',...args);}
}

export class IntOp extends Item {
  intVar: VariableBase;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.intVar=new VariableBase(this.prefix()+'/intVar');
  }
  attachedToDefiningVar(a1: Container<Item>): boolean {return this.callMethod('attachedToDefiningVar',a1);}
  coupled(): boolean {return this.callMethod('coupled');}
  description(...args: string[]): string {return this.callMethod('description',...args);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  intVarOffset(...args: number[]): number {return this.callMethod('intVarOffset',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  pack(a1: classdesc__pack_t,a2: string): void {return this.callMethod('pack',a1,a2);}
  ports(a1: number): object {return this.callMethod('ports',a1);}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  toggleCoupled(): boolean {return this.callMethod('toggleCoupled');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  unpack(a1: classdesc__pack_t,a2: string): void {return this.callMethod('unpack',a1,a2);}
  valueId(): string {return this.callMethod('valueId');}
}

export class LassoBox extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  angle(...args: number[]): number {return this.callMethod('angle',...args);}
  contains(a1: Wire): boolean {return this.callMethod('contains',a1);}
  x0(...args: number[]): number {return this.callMethod('x0',...args);}
  x1(...args: number[]): number {return this.callMethod('x1',...args);}
  y0(...args: number[]): number {return this.callMethod('y0',...args);}
  y1(...args: number[]): number {return this.callMethod('y1',...args);}
}

export class Lock extends Item {
  lockedIcon: SVGRenderer;
  lockedState: ravel__RavelState;
  unlockedIcon: SVGRenderer;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.lockedIcon=new SVGRenderer(this.prefix()+'/lockedIcon');
    this.lockedState=new ravel__RavelState(this.prefix()+'/lockedState');
    this.unlockedIcon=new SVGRenderer(this.prefix()+'/unlockedIcon');
  }
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  locked(): boolean {return this.callMethod('locked');}
  ravelInput(): Ravel {return this.callMethod('ravelInput');}
  toggleLocked(): void {return this.callMethod('toggleLocked');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
}

export class Minsky extends CppClass {
  canvas: Canvas;
  conversions: civita__Conversions;
  dimensions: Map<string,civita__Dimension>;
  equationDisplay: EquationDisplay;
  evalGodley: EvalGodley;
  flowVars: Sequence<number>;
  fontSampler: FontDisplay;
  godleyTab: GodleyTab;
  logVarList: Container<string>;
  model: Group;
  namedItems: Map<string,Item>;
  panopticon: Panopticon;
  parameterTab: ParameterTab;
  plotTab: PlotTab;
  stockVars: Sequence<number>;
  variableInstanceList: VariableInstanceList;
  variablePane: VariablePane;
  variableTab: VariableTab;
  variableValues: VariableValues;
  constructor(prefix: string){
    super(prefix);
    this.canvas=new Canvas(this.prefix()+'/canvas');
    this.conversions=new civita__Conversions(this.prefix()+'/conversions');
    this.dimensions=new Map<string,civita__Dimension>(this.prefix()+'/dimensions',civita__Dimension);
    this.equationDisplay=new EquationDisplay(this.prefix()+'/equationDisplay');
    this.evalGodley=new EvalGodley(this.prefix()+'/evalGodley');
    this.flowVars=new Sequence<number>(this.prefix()+'/flowVars');
    this.fontSampler=new FontDisplay(this.prefix()+'/fontSampler');
    this.godleyTab=new GodleyTab(this.prefix()+'/godleyTab');
    this.logVarList=new Container<string>(this.prefix()+'/logVarList');
    this.model=new Group(this.prefix()+'/model');
    this.namedItems=new Map<string,Item>(this.prefix()+'/namedItems',Item);
    this.panopticon=new Panopticon(this.prefix()+'/panopticon');
    this.parameterTab=new ParameterTab(this.prefix()+'/parameterTab');
    this.plotTab=new PlotTab(this.prefix()+'/plotTab');
    this.stockVars=new Sequence<number>(this.prefix()+'/stockVars');
    this.variableInstanceList=new VariableInstanceList(this.prefix()+'/variableInstanceList');
    this.variablePane=new VariablePane(this.prefix()+'/variablePane');
    this.variableTab=new VariableTab(this.prefix()+'/variableTab');
    this.variableValues=new VariableValues(this.prefix()+'/variableValues');
  }
  RKfunction(a1: number,a2: number,a3: number,a4: void): number {return this.callMethod('RKfunction',a1,a2,a3,a4);}
  addIntegral(): void {return this.callMethod('addIntegral');}
  assetClasses(): string[] {return this.callMethod('assetClasses');}
  autoLayout(): void {return this.callMethod('autoLayout');}
  autoSaveFile(): string {return this.callMethod('autoSaveFile');}
  availableOperations(): string[] {return this.callMethod('availableOperations');}
  availableOperationsMapping(): object[] {return this.callMethod('availableOperationsMapping');}
  balanceColumns(a1: GodleyIcon,a2: number,a3: GodleyIcon,a4: number): void {return this.callMethod('balanceColumns',a1,a2,a3,a4);}
  balanceDuplicateColumns(a1: GodleyIcon,a2: number): void {return this.callMethod('balanceDuplicateColumns',a1,a2);}
  checkEquationOrder(): boolean {return this.callMethod('checkEquationOrder');}
  checkMemAllocation(a1: number): boolean {return this.callMethod('checkMemAllocation',a1);}
  checkPushHistory(): void {return this.callMethod('checkPushHistory');}
  classifyOp(a1: string): string {return this.callMethod('classifyOp',a1);}
  clearAllMaps(...args: boolean[]): void {return this.callMethod('clearAllMaps',...args);}
  clearAllMapsTCL(): void {return this.callMethod('clearAllMapsTCL');}
  clearBusyCursor(): void {return this.callMethod('clearBusyCursor');}
  clearHistory(): void {return this.callMethod('clearHistory');}
  clipboardEmpty(): boolean {return this.callMethod('clipboardEmpty');}
  closeLogFile(): void {return this.callMethod('closeLogFile');}
  commandHook(a1: string,a2: number): boolean {return this.callMethod('commandHook',a1,a2);}
  constructEquations(): void {return this.callMethod('constructEquations');}
  convertVarType(a1: string,a2: string): void {return this.callMethod('convertVarType',a1,a2);}
  copy(): void {return this.callMethod('copy');}
  cut(): void {return this.callMethod('cut');}
  cycleCheck(): boolean {return this.callMethod('cycleCheck');}
  defaultFont(...args: any[]): string {return this.callMethod('defaultFont',...args);}
  definingVar(a1: string): object {return this.callMethod('definingVar',a1);}
  deleteAllUnits(): void {return this.callMethod('deleteAllUnits');}
  deltaT(): number {return this.callMethod('deltaT');}
  dimensionalAnalysis(): void {return this.callMethod('dimensionalAnalysis');}
  displayErrorItem(a1: Item): void {return this.callMethod('displayErrorItem',a1);}
  displayStyle(...args: string[]): string {return this.callMethod('displayStyle',...args);}
  displayValues(...args: boolean[]): boolean {return this.callMethod('displayValues',...args);}
  doPushHistory(...args: boolean[]): boolean {return this.callMethod('doPushHistory',...args);}
  ecolabVersion(): string {return this.callMethod('ecolabVersion');}
  edited(): boolean {return this.callMethod('edited');}
  epsAbs(...args: number[]): number {return this.callMethod('epsAbs',...args);}
  epsRel(...args: number[]): number {return this.callMethod('epsRel',...args);}
  evalEquations(): void {return this.callMethod('evalEquations');}
  exportAllPlotsAsCSV(a1: string): void {return this.callMethod('exportAllPlotsAsCSV',a1);}
  exportSchema(a1: string,a2: number): void {return this.callMethod('exportSchema',a1,a2);}
  fileVersion(...args: string[]): string {return this.callMethod('fileVersion',...args);}
  fontScale(...args: any[]): number {return this.callMethod('fontScale',...args);}
  garbageCollect(): void {return this.callMethod('garbageCollect');}
  getCommandData(a1: string): string {return this.callMethod('getCommandData',a1);}
  implicit(...args: boolean[]): boolean {return this.callMethod('implicit',...args);}
  importDuplicateColumn(a1: GodleyTable,a2: number): void {return this.callMethod('importDuplicateColumn',a1,a2);}
  importVensim(a1: string): void {return this.callMethod('importVensim',a1);}
  imposeDimensions(): void {return this.callMethod('imposeDimensions');}
  initGodleys(): void {return this.callMethod('initGodleys');}
  inputWired(a1: string): boolean {return this.callMethod('inputWired',a1);}
  insertGroupFromFile(a1: string): void {return this.callMethod('insertGroupFromFile',a1);}
  jacobian(a1: number,a2: number,a3: number,a4: number,a5: void): number {return this.callMethod('jacobian',a1,a2,a3,a4,a5);}
  lastT(...args: number[]): number {return this.callMethod('lastT',...args);}
  latex(a1: string,a2: boolean): void {return this.callMethod('latex',a1,a2);}
  latex2pango(a1: string): string {return this.callMethod('latex2pango',a1);}
  listAllInstances(): void {return this.callMethod('listAllInstances');}
  listFonts(): string[] {return this.callMethod('listFonts');}
  load(a1: string): void {return this.callMethod('load',a1);}
  makeVariablesConsistent(): void {return this.callMethod('makeVariablesConsistent');}
  markEdited(): void {return this.callMethod('markEdited');}
  matchingTableColumns(a1: GodleyIcon,a2: string): string[] {return this.callMethod('matchingTableColumns',a1,a2);}
  matlab(a1: string): void {return this.callMethod('matlab',a1);}
  maxHistory(...args: number[]): number {return this.callMethod('maxHistory',...args);}
  maxWaitMS(...args: number[]): number {return this.callMethod('maxWaitMS',...args);}
  message(a1: string): void {return this.callMethod('message',a1);}
  minskyVersion(...args: string[]): string {return this.callMethod('minskyVersion',...args);}
  multipleEquities(...args: any[]): boolean {return this.callMethod('multipleEquities',...args);}
  nSteps(...args: number[]): number {return this.callMethod('nSteps',...args);}
  nameCurrentItem(a1: string): void {return this.callMethod('nameCurrentItem',a1);}
  numOpArgs(a1: string): number {return this.callMethod('numOpArgs',a1);}
  openGroupInCanvas(): void {return this.callMethod('openGroupInCanvas');}
  openLogFile(a1: string): void {return this.callMethod('openLogFile',a1);}
  openModelInCanvas(): void {return this.callMethod('openModelInCanvas');}
  order(...args: number[]): number {return this.callMethod('order',...args);}
  paste(): void {return this.callMethod('paste');}
  physicalMem(): number {return this.callMethod('physicalMem');}
  popFlags(): void {return this.callMethod('popFlags');}
  populateMissingDimensions(): void {return this.callMethod('populateMissingDimensions');}
  populateMissingDimensionsFromVariable(a1: VariableValue): void {return this.callMethod('populateMissingDimensionsFromVariable',a1);}
  pushFlags(): void {return this.callMethod('pushFlags');}
  pushHistory(): boolean {return this.callMethod('pushHistory');}
  randomLayout(): void {return this.callMethod('randomLayout');}
  ravelExpired(): boolean {return this.callMethod('ravelExpired');}
  ravelVersion(): string {return this.callMethod('ravelVersion');}
  redrawAllGodleyTables(): void {return this.callMethod('redrawAllGodleyTables');}
  renameDimension(a1: string,a2: string): void {return this.callMethod('renameDimension',a1,a2);}
  renderAllPlotsAsSVG(a1: string): void {return this.callMethod('renderAllPlotsAsSVG',a1);}
  renderCanvasToEMF(a1: string): void {return this.callMethod('renderCanvasToEMF',a1);}
  renderCanvasToPDF(a1: string): void {return this.callMethod('renderCanvasToPDF',a1);}
  renderCanvasToPNG(a1: string): void {return this.callMethod('renderCanvasToPNG',a1);}
  renderCanvasToPS(a1: string): void {return this.callMethod('renderCanvasToPS',a1);}
  renderCanvasToSVG(a1: string): void {return this.callMethod('renderCanvasToSVG',a1);}
  renderEquationsToImage(a1: string): void {return this.callMethod('renderEquationsToImage',a1);}
  reset(): void {return this.callMethod('reset');}
  resetIfFlagged(): boolean {return this.callMethod('resetIfFlagged');}
  reset_flag(): boolean {return this.callMethod('reset_flag');}
  reverse(...args: boolean[]): boolean {return this.callMethod('reverse',...args);}
  rkreset(): void {return this.callMethod('rkreset');}
  rkstep(): void {return this.callMethod('rkstep');}
  runItemDeletedCallback(a1: Item): void {return this.callMethod('runItemDeletedCallback',a1);}
  running(...args: boolean[]): boolean {return this.callMethod('running',...args);}
  save(a1: string): void {return this.callMethod('save',a1);}
  saveCanvasItemAsFile(a1: string): void {return this.callMethod('saveCanvasItemAsFile',a1);}
  saveGroupAsFile(a1: Group,a2: string): void {return this.callMethod('saveGroupAsFile',a1,a2);}
  saveSelectionAsFile(a1: string): void {return this.callMethod('saveSelectionAsFile',a1);}
  setAllDEmode(a1: boolean): void {return this.callMethod('setAllDEmode',a1);}
  setAutoSaveFile(a1: string): void {return this.callMethod('setAutoSaveFile',a1);}
  setBusyCursor(): void {return this.callMethod('setBusyCursor');}
  setGodleyDisplayValue(a1: boolean,a2: string): void {return this.callMethod('setGodleyDisplayValue',a1,a2);}
  setGodleyIconResource(a1: string): void {return this.callMethod('setGodleyIconResource',a1);}
  setGroupIconResource(a1: string): void {return this.callMethod('setGroupIconResource',a1);}
  setLockIconResource(a1: string,a2: string): void {return this.callMethod('setLockIconResource',a1,a2);}
  setRavelIconResource(a1: string): void {return this.callMethod('setRavelIconResource',a1);}
  simulationDelay(...args: number[]): number {return this.callMethod('simulationDelay',...args);}
  srand(a1: number): void {return this.callMethod('srand',a1);}
  step(): number[] {return this.callMethod('step');}
  stepMax(...args: number[]): number {return this.callMethod('stepMax',...args);}
  stepMin(...args: number[]): number {return this.callMethod('stepMin',...args);}
  t(...args: number[]): number {return this.callMethod('t',...args);}
  t0(...args: number[]): number {return this.callMethod('t0',...args);}
  timeUnit(...args: string[]): string {return this.callMethod('timeUnit',...args);}
  tmax(...args: number[]): number {return this.callMethod('tmax',...args);}
  triggerCheckMemAllocationCallback(): boolean {return this.callMethod('triggerCheckMemAllocationCallback');}
  undo(a1: number): number {return this.callMethod('undo',a1);}
  variableTypes(): string[] {return this.callMethod('variableTypes');}
}

export class Panopticon extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cheight(...args: number[]): number {return this.callMethod('cheight',...args);}
  cleft(...args: number[]): number {return this.callMethod('cleft',...args);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  ctop(...args: number[]): number {return this.callMethod('ctop',...args);}
  cwidth(...args: number[]): number {return this.callMethod('cwidth',...args);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(...args: number[]): number {return this.callMethod('height',...args);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(...args: number[]): number {return this.callMethod('width',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class ParameterTab extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  bottomRowMargin: Sequence<number>;
  item: Item;
  itemFocus: Item;
  itemVector: Sequence<Item>;
  rightColMargin: Sequence<number>;
  varAttrib: Sequence<string>;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.prefix()+'/bottomRowMargin');
    this.item=new Item(this.prefix()+'/item');
    this.itemFocus=new Item(this.prefix()+'/itemFocus');
    this.itemVector=new Sequence<Item>(this.prefix()+'/itemVector',Item);
    this.rightColMargin=new Sequence<number>(this.prefix()+'/rightColMargin');
    this.varAttrib=new Sequence<string>(this.prefix()+'/varAttrib');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cell(a1: number,a2: number): ecolab__Pango {return this.callMethod('cell',a1,a2);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): object {return this.callMethod('itemAt',a1,a2);}
  itemSelector(a1: Item): boolean {return this.callMethod('itemSelector',a1);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(): number {return this.callMethod('numCols');}
  numRows(): number {return this.callMethod('numRows');}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xItem(...args: number[]): number {return this.callMethod('xItem',...args);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  yItem(...args: number[]): number {return this.callMethod('yItem',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class PlotTab extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  bottomRowMargin: Sequence<number>;
  item: Item;
  itemFocus: Item;
  itemVector: Sequence<Item>;
  rightColMargin: Sequence<number>;
  varAttrib: Sequence<string>;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.prefix()+'/bottomRowMargin');
    this.item=new Item(this.prefix()+'/item');
    this.itemFocus=new Item(this.prefix()+'/itemFocus');
    this.itemVector=new Sequence<Item>(this.prefix()+'/itemVector',Item);
    this.rightColMargin=new Sequence<number>(this.prefix()+'/rightColMargin');
    this.varAttrib=new Sequence<string>(this.prefix()+'/varAttrib');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cell(a1: number,a2: number): ecolab__Pango {return this.callMethod('cell',a1,a2);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): object {return this.callMethod('itemAt',a1,a2);}
  itemSelector(a1: Item): boolean {return this.callMethod('itemSelector',a1);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(): number {return this.callMethod('numCols');}
  numRows(): number {return this.callMethod('numRows');}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  togglePlotDisplay(): void {return this.callMethod('togglePlotDisplay');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xItem(...args: number[]): number {return this.callMethod('xItem',...args);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  yItem(...args: number[]): number {return this.callMethod('yItem',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class PlotWidget extends Item {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  palette: Sequence<ecolab__Plot__LineStyle>;
  wire: Wire;
  xmaxVar: VariableValue;
  xminVar: VariableValue;
  xvars: Sequence<VariableValue>;
  y1maxVar: VariableValue;
  y1minVar: VariableValue;
  ymaxVar: VariableValue;
  yminVar: VariableValue;
  yvars: Sequence<VariableValue>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.palette=new Sequence<ecolab__Plot__LineStyle>(this.prefix()+'/palette',ecolab__Plot__LineStyle);
    this.wire=new Wire(this.prefix()+'/wire');
    this.xmaxVar=new VariableValue(this.prefix()+'/xmaxVar');
    this.xminVar=new VariableValue(this.prefix()+'/xminVar');
    this.xvars=new Sequence<VariableValue>(this.prefix()+'/xvars',VariableValue);
    this.y1maxVar=new VariableValue(this.prefix()+'/y1maxVar');
    this.y1minVar=new VariableValue(this.prefix()+'/y1minVar');
    this.ymaxVar=new VariableValue(this.prefix()+'/ymaxVar');
    this.yminVar=new VariableValue(this.prefix()+'/yminVar');
    this.yvars=new Sequence<VariableValue>(this.prefix()+'/yvars',VariableValue);
  }
  AssignSide(a1: number,a2: string): void {return this.callMethod('AssignSide',a1,a2);}
  Image(...args: any[]): string {return this.callMethod('Image',...args);}
  LabelPen(a1: number,a2: ecolab__cairo__Surface): void {return this.callMethod('LabelPen',a1,a2);}
  add(a1: number,a2: number,a3: number): void {return this.callMethod('add',a1,a2,a3);}
  addConstantCurves(): void {return this.callMethod('addConstantCurves');}
  addPlotPt(a1: number): void {return this.callMethod('addPlotPt',a1);}
  addPorts(): void {return this.callMethod('addPorts');}
  addPt(a1: number,a2: number,a3: number): void {return this.callMethod('addPt',a1,a2,a3);}
  assignSide(a1: number,a2: string): void {return this.callMethod('assignSide',a1,a2);}
  autoScale(): void {return this.callMethod('autoScale');}
  autoscale(...args: boolean[]): boolean {return this.callMethod('autoscale',...args);}
  cairoSurface(): minsky__dummy {return this.callMethod('cairoSurface');}
  clear(): void {return this.callMethod('clear');}
  clearPenAttributes(): void {return this.callMethod('clearPenAttributes');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  connectVar(a1: VariableValue,a2: number): void {return this.callMethod('connectVar',a1,a2);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  disconnectAllVars(): void {return this.callMethod('disconnectAllVars');}
  displayFontSize(...args: number[]): number {return this.callMethod('displayFontSize',...args);}
  displayNTicks(...args: number[]): number {return this.callMethod('displayNTicks',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  exp_threshold(...args: number[]): number {return this.callMethod('exp_threshold',...args);}
  exportAsCSV(...args: any[]): void {return this.callMethod('exportAsCSV',...args);}
  extendPalette(): void {return this.callMethod('extendPalette');}
  fontScale(...args: number[]): number {return this.callMethod('fontScale',...args);}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  grid(...args: boolean[]): boolean {return this.callMethod('grid',...args);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  image(a1: object): string {return this.callMethod('image',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  labelPen(a1: number,a2: string): void {return this.callMethod('labelPen',a1,a2);}
  labelheight(): number {return this.callMethod('labelheight');}
  leadingMarker(...args: boolean[]): boolean {return this.callMethod('leadingMarker',...args);}
  legend(...args: boolean[]): boolean {return this.callMethod('legend',...args);}
  legendFontSz(...args: number[]): number {return this.callMethod('legendFontSz',...args);}
  legendLeft(...args: number[]): number {return this.callMethod('legendLeft',...args);}
  legendOffset(...args: number[]): number {return this.callMethod('legendOffset',...args);}
  legendSide(...args: string[]): string {return this.callMethod('legendSide',...args);}
  legendSize(a1: number,a2: number,a3: number): void {return this.callMethod('legendSize',a1,a2,a3);}
  legendTop(...args: number[]): number {return this.callMethod('legendTop',...args);}
  lh(a1: number,a2: number): number {return this.callMethod('lh',a1,a2);}
  logx(...args: boolean[]): boolean {return this.callMethod('logx',...args);}
  logy(...args: boolean[]): boolean {return this.callMethod('logy',...args);}
  makeDisplayPlot(): void {return this.callMethod('makeDisplayPlot');}
  maxx(...args: number[]): number {return this.callMethod('maxx',...args);}
  maxy(...args: number[]): number {return this.callMethod('maxy',...args);}
  maxy1(...args: number[]): number {return this.callMethod('maxy1',...args);}
  minx(...args: number[]): number {return this.callMethod('minx',...args);}
  miny(...args: number[]): number {return this.callMethod('miny',...args);}
  miny1(...args: number[]): number {return this.callMethod('miny1',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  nxTicks(...args: number[]): number {return this.callMethod('nxTicks',...args);}
  nyTicks(...args: number[]): number {return this.callMethod('nyTicks',...args);}
  offx(...args: number[]): number {return this.callMethod('offx',...args);}
  offy(...args: number[]): number {return this.callMethod('offy',...args);}
  pack(a1: classdesc__pack_t): void {return this.callMethod('pack',a1);}
  percent(...args: boolean[]): boolean {return this.callMethod('percent',...args);}
  plot(a1: object): void {return this.callMethod('plot',a1);}
  plotOnTab(): boolean {return this.callMethod('plotOnTab');}
  plotTabDisplay(...args: boolean[]): boolean {return this.callMethod('plotTabDisplay',...args);}
  plotType(...args: string[]): string {return this.callMethod('plotType',...args);}
  plotWidgetCast(): PlotWidget {return this.callMethod('plotWidgetCast');}
  position(): number[] {return this.callMethod('position');}
  redraw(...args: any[]): boolean {return this.callMethod('redraw',...args);}
  redrawWithBounds(): void {return this.callMethod('redrawWithBounds');}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  scalePlot(): void {return this.callMethod('scalePlot');}
  setMinMax(): void {return this.callMethod('setMinMax');}
  subgrid(...args: boolean[]): boolean {return this.callMethod('subgrid',...args);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
  togglePlotTabDisplay(): void {return this.callMethod('togglePlotTabDisplay');}
  unpack(a1: classdesc__pack_t): void {return this.callMethod('unpack',a1);}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xlabel(...args: string[]): string {return this.callMethod('xlabel',...args);}
  xtickAngle(...args: number[]): number {return this.callMethod('xtickAngle',...args);}
  y1label(...args: string[]): string {return this.callMethod('y1label',...args);}
  ylabel(...args: string[]): string {return this.callMethod('ylabel',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class Port extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  checkUnits(): object {return this.callMethod('checkUnits');}
  combineInput(a1: number,a2: number): void {return this.callMethod('combineInput',a1,a2);}
  deleteWires(): void {return this.callMethod('deleteWires');}
  eraseWire(a1: Wire): void {return this.callMethod('eraseWire',a1);}
  identity(): number {return this.callMethod('identity');}
  input(): boolean {return this.callMethod('input');}
  item(): Item {return this.callMethod('item');}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  multiWireAllowed(): boolean {return this.callMethod('multiWireAllowed');}
  numWires(): number {return this.callMethod('numWires');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  value(): number {return this.callMethod('value');}
  wires(): Sequence<Wire> {return this.callMethod('wires');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
}

export class Ravel extends Item {
  axisDimensions: Map<string,civita__Dimension>;
  lockGroup: RavelLockGroup;
  popup: RavelPopup;
  svgRenderer: SVGRenderer;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.axisDimensions=new Map<string,civita__Dimension>(this.prefix()+'/axisDimensions',civita__Dimension);
    this.lockGroup=new RavelLockGroup(this.prefix()+'/lockGroup');
    this.popup=new RavelPopup(this.prefix()+'/popup');
    this.svgRenderer=new SVGRenderer(this.prefix()+'/svgRenderer');
  }
  adjustSlicer(a1: number): void {return this.callMethod('adjustSlicer',a1);}
  allSliceLabels(): string[] {return this.callMethod('allSliceLabels');}
  allSliceLabelsAxis(a1: number): string[] {return this.callMethod('allSliceLabelsAxis',a1);}
  applyState(a1: ravel__RavelState): void {return this.callMethod('applyState',a1);}
  broadcastStateToLockGroup(): void {return this.callMethod('broadcastStateToLockGroup');}
  description(): string {return this.callMethod('description');}
  dimension(a1: number): object {return this.callMethod('dimension',a1);}
  dimensionType(...args: any[]): string {return this.callMethod('dimensionType',...args);}
  dimensionUnitsFormat(...args: any[]): string {return this.callMethod('dimensionUnitsFormat',...args);}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayFilterCaliper(): boolean {return this.callMethod('displayFilterCaliper');}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  editorMode(...args: boolean[]): boolean {return this.callMethod('editorMode',...args);}
  exportAsCSV(a1: string): void {return this.callMethod('exportAsCSV',a1);}
  getState(): object {return this.callMethod('getState');}
  handleDescription(a1: number): string {return this.callMethod('handleDescription',a1);}
  handleNames(): string[] {return this.callMethod('handleNames');}
  handleSetReduction(a1: number,a2: string): void {return this.callMethod('handleSetReduction',a1,a2);}
  handleSortableByValue(): boolean {return this.callMethod('handleSortableByValue');}
  hypercube(): object {return this.callMethod('hypercube');}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  leaveLockGroup(): void {return this.callMethod('leaveLockGroup');}
  maxRank(): number {return this.callMethod('maxRank');}
  nextReduction(a1: string): void {return this.callMethod('nextReduction',a1);}
  numHandles(): number {return this.callMethod('numHandles');}
  numSliceLabels(a1: number): number {return this.callMethod('numSliceLabels',a1);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  pickSliceLabels(a1: number,a2: string[]): void {return this.callMethod('pickSliceLabels',a1,a2);}
  pickedSliceLabels(): string[] {return this.callMethod('pickedSliceLabels');}
  populateHypercube(a1: civita__Hypercube): void {return this.callMethod('populateHypercube',a1);}
  redistributeHandles(): void {return this.callMethod('redistributeHandles');}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resortHandleIfDynamic(): void {return this.callMethod('resortHandleIfDynamic');}
  selectedHandle(): number {return this.callMethod('selectedHandle');}
  setDescription(a1: string): void {return this.callMethod('setDescription',a1);}
  setDimension(...args: any[]): void {return this.callMethod('setDimension',...args);}
  setDisplayFilterCaliper(a1: boolean): boolean {return this.callMethod('setDisplayFilterCaliper',a1);}
  setHandleDescription(a1: number,a2: string): void {return this.callMethod('setHandleDescription',a1,a2);}
  setHandleSortOrder(a1: string,a2: number): string {return this.callMethod('setHandleSortOrder',a1,a2);}
  setRank(a1: number): void {return this.callMethod('setRank',a1);}
  setSortOrder(a1: string): string {return this.callMethod('setSortOrder',a1);}
  sortByValue(a1: string): void {return this.callMethod('sortByValue',a1);}
  sortOrder(): string {return this.callMethod('sortOrder');}
  toggleDisplayFilterCaliper(): boolean {return this.callMethod('toggleDisplayFilterCaliper');}
  toggleEditorMode(): void {return this.callMethod('toggleEditorMode');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
}

export class RavelLockGroup extends CppClass {
  handleLockInfo: Sequence<HandleLockInfo>;
  constructor(prefix: string){
    super(prefix);
    this.handleLockInfo=new Sequence<HandleLockInfo>(this.prefix()+'/handleLockInfo',HandleLockInfo);
  }
  addHandleInfo(a1: Ravel): void {return this.callMethod('addHandleInfo',a1);}
  addRavel(a1: Ravel): void {return this.callMethod('addRavel',a1);}
  allLockHandles(): string[] {return this.callMethod('allLockHandles');}
  broadcast(a1: Ravel): void {return this.callMethod('broadcast',a1);}
  colour(): number {return this.callMethod('colour');}
  handleNames(a1: number): string[] {return this.callMethod('handleNames',a1);}
  initialBroadcast(): void {return this.callMethod('initialBroadcast');}
  ravelNames(): string[] {return this.callMethod('ravelNames');}
  ravels(): Sequence<Ravel> {return this.callMethod('ravels');}
  removeFromGroup(a1: Ravel): void {return this.callMethod('removeFromGroup',a1);}
  setLockHandles(a1: string[]): void {return this.callMethod('setLockHandles',a1);}
  validateLockHandleInfo(): void {return this.callMethod('validateLockHandleInfo');}
}

export class RavelPopup extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseLeave(): void {return this.callMethod('mouseLeave');}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseOver(a1: number,a2: number): void {return this.callMethod('mouseOver',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class RenderNativeWindow extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.item=new Item(this.prefix()+'/item');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  position(): number[] {return this.callMethod('position');}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class SVGRenderer extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  height(): number {return this.callMethod('height');}
  render(a1: minsky__dummy): void {return this.callMethod('render',a1);}
  setResource(a1: string): void {return this.callMethod('setResource',a1);}
  width(): number {return this.callMethod('width');}
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
    this.bb=new BoundingBox(this.prefix()+'/bb');
    this.bookmarks=new Container<Bookmark>(this.prefix()+'/bookmarks',Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(this.prefix()+'/createdIOvariables',VariableBase);
    this.groups=new Sequence<Group>(this.prefix()+'/groups',Group);
    this.inVariables=new Sequence<VariableBase>(this.prefix()+'/inVariables',VariableBase);
    this.items=new Sequence<Item>(this.prefix()+'/items',Item);
    this.outVariables=new Sequence<VariableBase>(this.prefix()+'/outVariables',VariableBase);
    this.svgRenderer=new SVGRenderer(this.prefix()+'/svgRenderer');
    this.wires=new Sequence<Wire>(this.prefix()+'/wires',Wire);
  }
  RESTProcess(a1: minsky__dummy,a2: string): void {return this.callMethod('RESTProcess',a1,a2);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  accessibleVars(): string[] {return this.callMethod('accessibleVars');}
  addBookmark(a1: string): void {return this.callMethod('addBookmark',a1);}
  addBookmarkXY(a1: number,a2: number,a3: string): void {return this.callMethod('addBookmarkXY',a1,a2,a3);}
  addGroup(a1: Group): object {return this.callMethod('addGroup',a1);}
  addInputVar(): void {return this.callMethod('addInputVar');}
  addItem(...args: any[]): object {return this.callMethod('addItem',...args);}
  addOutputVar(): void {return this.callMethod('addOutputVar');}
  addWire(...args: any[]): object {return this.callMethod('addWire',...args);}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustWiresGroup(a1: Wire): void {return this.callMethod('adjustWiresGroup',a1);}
  arguments(): string {return this.callMethod('arguments');}
  attachedToDefiningVar(...args: Container<Item>[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  autoLayout(): void {return this.callMethod('autoLayout');}
  bookmark(...args: boolean[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bookmarkList(): string[] {return this.callMethod('bookmarkList');}
  bottom(): number {return this.callMethod('bottom');}
  cBounds(): number[] {return this.callMethod('cBounds');}
  checkAddIORegion(a1: Item): void {return this.callMethod('checkAddIORegion',a1);}
  checkUnits(): object {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  clear(): void {return this.callMethod('clear');}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): Item {return this.callMethod('clone');}
  closestInPort(a1: number,a2: number): object {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): object {return this.callMethod('closestOutPort',a1,a2);}
  computeDisplayZoom(): number {return this.callMethod('computeDisplayZoom');}
  computeRelZoom(): void {return this.callMethod('computeRelZoom');}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  contentBounds(a1: number,a2: number,a3: number,a4: number): number {return this.callMethod('contentBounds',a1,a2,a3,a4);}
  copy(): object {return this.callMethod('copy');}
  copyUnowned(): object {return this.callMethod('copyUnowned');}
  corners(): object[] {return this.callMethod('corners');}
  defaultExtension(): string {return this.callMethod('defaultExtension');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteBookmark(a1: number): void {return this.callMethod('deleteBookmark',a1);}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  deleteItem(a1: Item): void {return this.callMethod('deleteItem',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayContents(): boolean {return this.callMethod('displayContents');}
  displayContentsChanged(): boolean {return this.callMethod('displayContentsChanged');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: minsky__dummy,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  displayZoom(...args: number[]): number {return this.callMethod('displayZoom',...args);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  draw1edge(a1: Sequence<VariableBase>,a2: minsky__dummy,a3: number): void {return this.callMethod('draw1edge',a1,a2,a3);}
  drawEdgeVariables(a1: minsky__dummy): void {return this.callMethod('drawEdgeVariables',a1);}
  drawIORegion(a1: minsky__dummy): void {return this.callMethod('drawIORegion',a1);}
  drawPorts(a1: minsky__dummy): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: minsky__dummy): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: minsky__dummy): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  edgeScale(): number {return this.callMethod('edgeScale');}
  empty(): boolean {return this.callMethod('empty');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  ensureGroupInserted(a1: Group): void {return this.callMethod('ensureGroupInserted',a1);}
  ensureItemInserted(a1: Item): void {return this.callMethod('ensureItemInserted',a1);}
  findGroup(a1: Group): object {return this.callMethod('findGroup',a1);}
  findItem(a1: Item): object {return this.callMethod('findItem',a1);}
  findWire(a1: Wire): object {return this.callMethod('findWire',a1);}
  flip(): void {return this.callMethod('flip');}
  flipContents(): void {return this.callMethod('flipContents');}
  formula(): string {return this.callMethod('formula');}
  globalGroup(): Group {return this.callMethod('globalGroup');}
  gotoBookmark(a1: number): void {return this.callMethod('gotoBookmark',a1);}
  gotoBookmark_b(a1: Bookmark): void {return this.callMethod('gotoBookmark_b',a1);}
  height(): number {return this.callMethod('height');}
  higher(a1: Group): boolean {return this.callMethod('higher',a1);}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  inIORegion(a1: number,a2: number): string {return this.callMethod('inIORegion',a1,a2);}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  insertGroup(a1: Group): void {return this.callMethod('insertGroup',a1);}
  insertItem(a1: Item): void {return this.callMethod('insertItem',a1);}
  ioVar(): boolean {return this.callMethod('ioVar');}
  itemPtrFromThis(): object {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: boolean[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  left(): number {return this.callMethod('left');}
  level(): number {return this.callMethod('level');}
  localZoom(): number {return this.callMethod('localZoom');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  makeSubroutine(): void {return this.callMethod('makeSubroutine');}
  margins(a1: number,a2: number): void {return this.callMethod('margins',a1,a2);}
  marginsV(): number[] {return this.callMethod('marginsV');}
  minimalEnclosingGroup(a1: number,a2: number,a3: number,a4: number,a5: Item): Group {return this.callMethod('minimalEnclosingGroup',a1,a2,a3,a4,a5);}
  mouseFocus(...args: boolean[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveContents(a1: Group): void {return this.callMethod('moveContents',a1);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  name(): string {return this.callMethod('name');}
  nocycles(): boolean {return this.callMethod('nocycles');}
  normaliseGroupRefs(a1: Group): void {return this.callMethod('normaliseGroupRefs',a1);}
  numGroups(): number {return this.callMethod('numGroups');}
  numItems(): number {return this.callMethod('numItems');}
  numWires(): number {return this.callMethod('numWires');}
  onBorder(...args: boolean[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: boolean[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): object {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  randomLayout(): void {return this.callMethod('randomLayout');}
  relZoom(...args: number[]): number {return this.callMethod('relZoom',...args);}
  removeDisplayPlot(): void {return this.callMethod('removeDisplayPlot');}
  removeGroup(a1: Group): object {return this.callMethod('removeGroup',a1);}
  removeItem(a1: Item): object {return this.callMethod('removeItem',a1);}
  removeWire(a1: Wire): object {return this.callMethod('removeWire',a1);}
  resize(a1: LassoBox): void {return this.callMethod('resize',a1);}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  resizeOnContents(): void {return this.callMethod('resizeOnContents');}
  right(): number {return this.callMethod('right');}
  rotFactor(): number {return this.callMethod('rotFactor');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): object {return this.callMethod('select',a1,a2);}
  selected(...args: boolean[]): boolean {return this.callMethod('selected',...args);}
  setZoom(a1: number): void {return this.callMethod('setZoom',a1);}
  splitBoundaryCrossingWires(): void {return this.callMethod('splitBoundaryCrossingWires');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
  toggleItemMembership(a1: Item): void {return this.callMethod('toggleItemMembership',a1);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  uniqueItems(...args: Container<void>[]): boolean {return this.callMethod('uniqueItems',...args);}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  value(): number {return this.callMethod('value');}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
}

export class Sheet extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
  }
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  showSlice(...args: string[]): string {return this.callMethod('showSlice',...args);}
}

export class SwitchIcon extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
  }
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  flip(): void {return this.callMethod('flip');}
  flipped(...args: boolean[]): boolean {return this.callMethod('flipped',...args);}
  numCases(): number {return this.callMethod('numCases');}
  setNumCases(a1: number): void {return this.callMethod('setNumCases',a1);}
  switchValue(): number {return this.callMethod('switchValue');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  value(): number {return this.callMethod('value');}
}

export class Units extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
  }
  latexStr(): string {return this.callMethod('latexStr');}
  normalise(): void {return this.callMethod('normalise');}
  str(): string {return this.callMethod('str');}
}

export class UserFunction extends Item {
  argNames: Sequence<string>;
  argVals: Sequence<number>;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
    this.argNames=new Sequence<string>(this.prefix()+'/argNames');
    this.argVals=new Sequence<number>(this.prefix()+'/argVals');
  }
  compile(): void {return this.callMethod('compile');}
  create(a1: string): UserFunction {return this.callMethod('create',a1);}
  description(...args: any[]): string {return this.callMethod('description',...args);}
  displayTooltip(a1: minsky__dummy,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  evaluate(a1: number,a2: number): number {return this.callMethod('evaluate',a1,a2);}
  expression(...args: string[]): string {return this.callMethod('expression',...args);}
  name(): string {return this.callMethod('name');}
  nextId(...args: number[]): number {return this.callMethod('nextId',...args);}
  symbolNames(): string[] {return this.callMethod('symbolNames');}
  units(a1: boolean): object {return this.callMethod('units',a1);}
}

export class VariableInstanceList extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  gotoInstance(a1: number): void {return this.callMethod('gotoInstance',a1);}
  names(): string[] {return this.callMethod('names');}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.prefix()+'/bottomRowMargin');
    this.item=new Item(this.prefix()+'/item');
    this.rightColMargin=new Sequence<number>(this.prefix()+'/rightColMargin');
    this.selection=new Container<string>(this.prefix()+'/selection');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cell(a1: number,a2: number): VariablePaneCell {return this.callMethod('cell',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  deselect(a1: string): void {return this.callMethod('deselect',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mousePressed(...args: boolean[]): boolean {return this.callMethod('mousePressed',...args);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(): number {return this.callMethod('numCols');}
  numRows(): number {return this.callMethod('numRows');}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  select(a1: string): void {return this.callMethod('select',a1);}
  shift(...args: boolean[]): boolean {return this.callMethod('shift',...args);}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  update(): void {return this.callMethod('update');}
  updateWithHeight(a1: number): void {return this.callMethod('updateWithHeight',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class VariablePaneCell extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  emplace(): void {return this.callMethod('emplace');}
  height(): number {return this.callMethod('height');}
  reset(a1: minsky__dummy): void {return this.callMethod('reset',a1);}
  show(): void {return this.callMethod('show');}
  width(): number {return this.callMethod('width');}
}

export class VariableTab extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  bottomRowMargin: Sequence<number>;
  item: Item;
  itemFocus: Item;
  itemVector: Sequence<Item>;
  rightColMargin: Sequence<number>;
  varAttrib: Sequence<string>;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix()+'/backgroundColour');
    this.bottomRowMargin=new Sequence<number>(this.prefix()+'/bottomRowMargin');
    this.item=new Item(this.prefix()+'/item');
    this.itemFocus=new Item(this.prefix()+'/itemFocus');
    this.itemVector=new Sequence<Item>(this.prefix()+'/itemVector',Item);
    this.rightColMargin=new Sequence<number>(this.prefix()+'/rightColMargin');
    this.varAttrib=new Sequence<string>(this.prefix()+'/varAttrib');
    this.wire=new Wire(this.prefix()+'/wire');
  }
  cell(a1: number,a2: number): ecolab__Pango {return this.callMethod('cell',a1,a2);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(a1: number,a2: number): boolean {return this.callMethod('getItemAt',a1,a2);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): boolean {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): object {return this.callMethod('itemAt',a1,a2);}
  itemSelector(a1: Item): boolean {return this.callMethod('itemSelector',a1);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(a1: number,a2: number): void {return this.callMethod('mouseDown',a1,a2);}
  mouseMove(a1: number,a2: number): void {return this.callMethod('mouseMove',a1,a2);}
  mouseUp(a1: number,a2: number): void {return this.callMethod('mouseUp',a1,a2);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(): number {return this.callMethod('numCols');}
  numRows(): number {return this.callMethod('numRows');}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): number[] {return this.callMethod('position');}
  redraw(a1: number,a2: number,a3: number,a4: number): boolean {return this.callMethod('redraw',a1,a2,a3,a4);}
  registerImage(): void {return this.callMethod('registerImage');}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(a1: string): void {return this.callMethod('renderToEMF',a1);}
  renderToPDF(a1: string): void {return this.callMethod('renderToPDF',a1);}
  renderToPNG(a1: string): void {return this.callMethod('renderToPNG',a1);}
  renderToPS(a1: string): void {return this.callMethod('renderToPS',a1);}
  renderToSVG(a1: string): void {return this.callMethod('renderToSVG',a1);}
  reportDrawTime(a1: number): void {return this.callMethod('reportDrawTime',a1);}
  requestRedraw(): void {return this.callMethod('requestRedraw');}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: minsky__dummy): object {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xItem(...args: number[]): number {return this.callMethod('xItem',...args);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  yItem(...args: number[]): number {return this.callMethod('yItem',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class VariableValue extends CppClass {
  csvDialog: CSVDialog;
  tensorInit: civita__TensorVal;
  units: Units;
  constructor(prefix: string){
    super(prefix);
    this.csvDialog=new CSVDialog(this.prefix()+'/csvDialog');
    this.tensorInit=new civita__TensorVal(this.prefix()+'/tensorInit');
    this.units=new Units(this.prefix()+'/units');
  }
  allocValue(): VariableValue {return this.callMethod('allocValue');}
  atHCIndex(a1: number): number {return this.callMethod('atHCIndex',a1);}
  begin(): number {return this.callMethod('begin');}
  end(): number {return this.callMethod('end');}
  exportAsCSV(a1: string,a2: string): void {return this.callMethod('exportAsCSV',a1,a2);}
  godleyOverridden(...args: boolean[]): boolean {return this.callMethod('godleyOverridden',...args);}
  hypercube(...args: any[]): civita__Hypercube {return this.callMethod('hypercube',...args);}
  idx(): number {return this.callMethod('idx');}
  idxInRange(): boolean {return this.callMethod('idxInRange');}
  imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): void {return this.callMethod('imposeDimensions',a1);}
  index(...args: any[]): civita__Index {return this.callMethod('index',...args);}
  init(...args: string[]): string {return this.callMethod('init',...args);}
  isFlowVar(): boolean {return this.callMethod('isFlowVar');}
  isZero(): boolean {return this.callMethod('isZero');}
  lhs(): boolean {return this.callMethod('lhs');}
  name(...args: string[]): string {return this.callMethod('name',...args);}
  rank(): number {return this.callMethod('rank');}
  reset_idx(): void {return this.callMethod('reset_idx');}
  setArgument(a1: civita__ITensor,a2: string,a3: number): void {return this.callMethod('setArgument',a1,a2,a3);}
  setArguments(...args: any[]): void {return this.callMethod('setArguments',...args);}
  setUnits(a1: string): void {return this.callMethod('setUnits',a1);}
  setValue(...args: any[]): number {return this.callMethod('setValue',...args);}
  shape(): number[] {return this.callMethod('shape');}
  size(): number {return this.callMethod('size');}
  sliderVisible(...args: boolean[]): boolean {return this.callMethod('sliderVisible',...args);}
  temp(): boolean {return this.callMethod('temp');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  unitsCached(...args: boolean[]): boolean {return this.callMethod('unitsCached',...args);}
  value(): number {return this.callMethod('value');}
  valueAt(a1: number): number {return this.callMethod('valueAt',a1);}
  valueId(): string {return this.callMethod('valueId');}
}

export class VariableValues extends Map<string,VariableValue> {
  constructor(prefix: string|Map<string,VariableValue>){
    if (typeof prefix==='string')
      super(prefix,VariableValue)
    else
      super(prefix.prefix(),VariableValue)
  }
  clear(): void {return this.callMethod('clear');}
  initValue(...args: any[]): object {return this.callMethod('initValue',...args);}
  newName(a1: string): string {return this.callMethod('newName',a1);}
  reset(): void {return this.callMethod('reset');}
  resetUnitsCache(): void {return this.callMethod('resetUnitsCache');}
  resetValue(a1: VariableValue): void {return this.callMethod('resetValue',a1);}
  validEntries(): boolean {return this.callMethod('validEntries');}
}

export class Wire extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: boolean[]): boolean {return this.callMethod('bookmark',...args);}
  coords(...args: any[]): number[] {return this.callMethod('coords',...args);}
  deleteHandle(a1: number,a2: number): void {return this.callMethod('deleteHandle',a1,a2);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  draw(a1: minsky__dummy): void {return this.callMethod('draw',a1);}
  editHandle(a1: number,a2: number,a3: number): void {return this.callMethod('editHandle',a1,a2,a3);}
  from(): object {return this.callMethod('from');}
  insertHandle(a1: number,a2: number,a3: number): void {return this.callMethod('insertHandle',a1,a2,a3);}
  mouseFocus(...args: boolean[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveIntoGroup(a1: Group): void {return this.callMethod('moveIntoGroup',a1);}
  moveToPorts(a1: Port,a2: Port): void {return this.callMethod('moveToPorts',a1,a2);}
  near(a1: number,a2: number): boolean {return this.callMethod('near',a1,a2);}
  nearestHandle(a1: number,a2: number): number {return this.callMethod('nearestHandle',a1,a2);}
  selected(...args: boolean[]): boolean {return this.callMethod('selected',...args);}
  split(): void {return this.callMethod('split');}
  storeCairoCoords(a1: minsky__dummy): void {return this.callMethod('storeCairoCoords',a1);}
  straighten(): void {return this.callMethod('straighten');}
  to(): object {return this.callMethod('to');}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  units(a1: boolean): object {return this.callMethod('units',a1);}
  visible(): boolean {return this.callMethod('visible');}
}

export class civita__Conversions extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super(prefix.prefix())
  }
  convert(a1: number,a2: string,a3: string): number {return this.callMethod('convert',a1,a2,a3);}
}

export class civita__Dimension extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  type(...args: string[]): string {return this.callMethod('type',...args);}
  units(...args: string[]): string {return this.callMethod('units',...args);}
}

export class civita__Hypercube extends CppClass {
  xvectors: Sequence<civita__XVector>;
  constructor(prefix: string){
    super(prefix);
    this.xvectors=new Sequence<civita__XVector>(this.prefix()+'/xvectors',civita__XVector);
  }
  dimLabels(): string[] {return this.callMethod('dimLabels');}
  dims(...args: any[]): number[] {return this.callMethod('dims',...args);}
  logNumElements(): number {return this.callMethod('logNumElements');}
  numElements(): number {return this.callMethod('numElements');}
  rank(): number {return this.callMethod('rank');}
  splitIndex(a1: number): number[] {return this.callMethod('splitIndex',a1);}
}

export class civita__ITensor extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  atHCIndex(a1: number): number {return this.callMethod('atHCIndex',a1);}
  hypercube(...args: any[]): civita__Hypercube {return this.callMethod('hypercube',...args);}
  imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): void {return this.callMethod('imposeDimensions',a1);}
  index(): civita__Index {return this.callMethod('index');}
  rank(): number {return this.callMethod('rank');}
  setArgument(a1: civita__ITensor,a2: string,a3: number): void {return this.callMethod('setArgument',a1,a2,a3);}
  setArguments(...args: any[]): void {return this.callMethod('setArguments',...args);}
  shape(): number[] {return this.callMethod('shape');}
  size(): number {return this.callMethod('size');}
}

export class civita__Index extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  begin(): object {return this.callMethod('begin');}
  clear(): void {return this.callMethod('clear');}
  empty(): boolean {return this.callMethod('empty');}
  end(): object {return this.callMethod('end');}
  linealOffset(a1: number): number {return this.callMethod('linealOffset',a1);}
  size(): number {return this.callMethod('size');}
}

export class civita__TensorVal extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  allocVal(): void {return this.callMethod('allocVal');}
  atHCIndex(a1: number): number {return this.callMethod('atHCIndex',a1);}
  begin(): number {return this.callMethod('begin');}
  end(): number {return this.callMethod('end');}
  hypercube(...args: any[]): civita__Hypercube {return this.callMethod('hypercube',...args);}
  imposeDimensions(a1: Container<Pair<string,civita__Dimension>>): void {return this.callMethod('imposeDimensions',a1);}
  index(...args: any[]): civita__Index {return this.callMethod('index',...args);}
  rank(): number {return this.callMethod('rank');}
  setArgument(a1: civita__ITensor,a2: string,a3: number): void {return this.callMethod('setArgument',a1,a2,a3);}
  setArguments(...args: any[]): void {return this.callMethod('setArguments',...args);}
  shape(): number[] {return this.callMethod('shape');}
  size(): number {return this.callMethod('size');}
  timestamp(): object {return this.callMethod('timestamp');}
  updateTimestamp(): void {return this.callMethod('updateTimestamp');}
}

export class civita__XVector extends CppClass {
  dimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dimension=new civita__Dimension(this.prefix()+'/dimension');
  }
  checkThisType(): boolean {return this.callMethod('checkThisType');}
  imposeDimension(): void {return this.callMethod('imposeDimension');}
  name(...args: string[]): string {return this.callMethod('name',...args);}
  push_back(...args: any[]): void {return this.callMethod('push_back',...args);}
  timeFormat(): string {return this.callMethod('timeFormat');}
}

export class ecolab__Plot__LineStyle extends CppClass {
  colour: ecolab__cairo__Colour;
  constructor(prefix: string){
    super(prefix);
    this.colour=new ecolab__cairo__Colour(this.prefix()+'/colour');
  }
  dashPattern(): number[] {return this.callMethod('dashPattern');}
  dashStyle(...args: string[]): string {return this.callMethod('dashStyle',...args);}
  width(...args: number[]): number {return this.callMethod('width',...args);}
}

export class ecolab__cairo__Colour extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  a(...args: number[]): number {return this.callMethod('a',...args);}
  b(...args: number[]): number {return this.callMethod('b',...args);}
  g(...args: number[]): number {return this.callMethod('g',...args);}
  r(...args: number[]): number {return this.callMethod('r',...args);}
}

export class ravel__HandleState extends CppClass {
  customOrder: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.customOrder=new Sequence<string>(this.prefix()+'/customOrder');
  }
  collapsed(...args: boolean[]): boolean {return this.callMethod('collapsed',...args);}
  description(...args: string[]): string {return this.callMethod('description',...args);}
  displayFilterCaliper(...args: boolean[]): boolean {return this.callMethod('displayFilterCaliper',...args);}
  format(...args: string[]): string {return this.callMethod('format',...args);}
  maxLabel(...args: string[]): string {return this.callMethod('maxLabel',...args);}
  minLabel(...args: string[]): string {return this.callMethod('minLabel',...args);}
  order(...args: string[]): string {return this.callMethod('order',...args);}
  reductionOp(...args: string[]): string {return this.callMethod('reductionOp',...args);}
  sliceLabel(...args: string[]): string {return this.callMethod('sliceLabel',...args);}
  x(...args: number[]): number {return this.callMethod('x',...args);}
  y(...args: number[]): number {return this.callMethod('y',...args);}
}

export class ravel__RavelState extends CppClass {
  handleStates: Sequence<ravel__HandleState>;
  outputHandles: Sequence<string>;
  constructor(prefix: string){
    super(prefix);
    this.handleStates=new Sequence<ravel__HandleState>(this.prefix()+'/handleStates',ravel__HandleState);
    this.outputHandles=new Sequence<string>(this.prefix()+'/outputHandles');
  }
  clear(): void {return this.callMethod('clear');}
  empty(): boolean {return this.callMethod('empty');}
  radius(...args: number[]): number {return this.callMethod('radius',...args);}
}

export var minsky=new Minsky('/minsky');
