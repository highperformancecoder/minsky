/*
This is a built file, please do not edit.
See RESTService/typescriptAPI for more information.
*/

import {CppClass, Sequence, Container, Map} from './backend';

class boost__geometry__model__d2__point_xy {}
class classdesc_json_pack_t {}
class classdesc_TCL_obj_t {}
class ecolab__cairo__Surface {}
class ecolab__Pango {}
class ecolab__TCL_args {}
class cairo_t {}
class __function__ {}

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
  bottom(...args: any[]): number {return this.callMethod('bottom',...args);}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  height(...args: any[]): number {return this.callMethod('height',...args);}
  left(...args: any[]): number {return this.callMethod('left',...args);}
  right(...args: any[]): number {return this.callMethod('right',...args);}
  top(...args: any[]): number {return this.callMethod('top',...args);}
  update(...args: any[]): void {return this.callMethod('update',...args);}
  valid(...args: any[]): boolean {return this.callMethod('valid',...args);}
  width(...args: any[]): number {return this.callMethod('width',...args);}
}

export class CSVDialog extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  spec: DataSpec;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.spec=new DataSpec(this.prefix+'spec');
  }
  colWidth(...args: number[]): number {return this.callMethod('colWidth',...args);}
  columnOver(a1: number): number {return this.callMethod('columnOver',a1);}
  copyHeaderRowToDimNames(a1: number): void {return this.callMethod('copyHeaderRowToDimNames',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  flashNameRow(...args: number[]): boolean {return this.callMethod('flashNameRow',...args);}
  guessSpecAndLoadFile(): void {return this.callMethod('guessSpecAndLoadFile');}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  headerForCol(a1: number): string {return this.callMethod('headerForCol',a1);}
  loadFile(): void {return this.callMethod('loadFile');}
  loadFileFromName(a1: string): void {return this.callMethod('loadFileFromName',a1);}
  loadWebFile(a1: string): string {return this.callMethod('loadWebFile',a1);}
  parseLines(): Sequence<Sequence<string>> {return this.callMethod('parseLines');}
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
  reportFromFile(a1: string,a2: string): void {return this.callMethod('reportFromFile',a1,a2);}
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowOver(a1: number): number {return this.callMethod('rowOver',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  tableWidth(): number {return this.callMethod('tableWidth');}
  url(...args: string[]): string {return this.callMethod('url',...args);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.item=new Item(this.prefix+'item');
    this.itemFocus=new Item(this.prefix+'itemFocus');
    this.itemVector=new Sequence<Item>(prefix,Item);
    this.lasso=new LassoBox(this.prefix+'lasso');
    this.model=new Group(this.prefix+'model');
    this.selection=new Selection(this.prefix+'selection');
    this.updateRegion=new LassoBox(this.prefix+'updateRegion');
    this.wire=new Wire(this.prefix+'wire');
    this.wireFocus=new Wire(this.prefix+'wireFocus');
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
  closestInPort(a1: number,a2: number): Port {return this.callMethod('closestInPort',a1,a2);}
  controlMouseDown(...args: any[]): void {return this.callMethod('controlMouseDown',...args);}
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
  focusFollowsMouse(...args: number[]): boolean {return this.callMethod('focusFollowsMouse',...args);}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getWireAt(...args: any[]): void {return this.callMethod('getWireAt',...args);}
  groupSelection(): void {return this.callMethod('groupSelection');}
  handleSelected(...args: number[]): number {return this.callMethod('handleSelected',...args);}
  hasScrollBars(...args: any[]): boolean {return this.callMethod('hasScrollBars',...args);}
  itemAt(a1: number,a2: number): Item {return this.callMethod('itemAt',a1,a2);}
  itemIndicator(...args: number[]): boolean {return this.callMethod('itemIndicator',...args);}
  keyPress(...args: any[]): boolean {return this.callMethod('keyPress',...args);}
  lassoMode(...args: string[]): string {return this.callMethod('lassoMode',...args);}
  lockRavelsInSelection(): void {return this.callMethod('lockRavelsInSelection');}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  openGroupInCanvas(a1: Item): void {return this.callMethod('openGroupInCanvas',a1);}
  position(...args: any[]): Sequence<number> {return this.callMethod('position',...args);}
  pushDefiningVarsToTab(): void {return this.callMethod('pushDefiningVarsToTab');}
  recentre(): void {return this.callMethod('recentre');}
  redraw(...args: any[]): boolean {return this.callMethod('redraw',...args);}
  redrawAll(...args: number[]): boolean {return this.callMethod('redrawAll',...args);}
  redrawRequested(): boolean {return this.callMethod('redrawRequested');}
  redrawUpdateRegion(): boolean {return this.callMethod('redrawUpdateRegion');}
  redrawWithBounds(): void {return this.callMethod('redrawWithBounds');}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
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
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  zoom(...args: any[]): void {return this.callMethod('zoom',...args);}
  zoomToDisplay(): void {return this.callMethod('zoomToDisplay');}
}

export class DataSpec extends CppClass {
  dataCols: Container<number>;
  dimensionCols: Container<number>;
  dimensionNames: Sequence<string>;
  dimensions: Sequence<civita__Dimension>;
  horizontalDimension: civita__Dimension;
  constructor(prefix: string){
    super(prefix);
    this.dataCols=new Container<number>(prefix);
    this.dimensionCols=new Container<number>(prefix);
    this.dimensionNames=new Sequence<string>(prefix);
    this.dimensions=new Sequence<civita__Dimension>(prefix,civita__Dimension);
    this.horizontalDimension=new civita__Dimension(this.prefix+'horizontalDimension');
  }
  columnar(...args: number[]): boolean {return this.callMethod('columnar',...args);}
  dataColOffset(...args: number[]): number {return this.callMethod('dataColOffset',...args);}
  dataRowOffset(...args: number[]): number {return this.callMethod('dataRowOffset',...args);}
  decSeparator(...args: number[]): number {return this.callMethod('decSeparator',...args);}
  duplicateKeyAction(...args: string[]): string {return this.callMethod('duplicateKeyAction',...args);}
  escape(...args: number[]): number {return this.callMethod('escape',...args);}
  guessFromFile(a1: string): void {return this.callMethod('guessFromFile',a1);}
  guessFromStream(a1: std__istream): void {return this.callMethod('guessFromStream',a1);}
  headerRow(...args: number[]): number {return this.callMethod('headerRow',...args);}
  horizontalDimName(...args: string[]): string {return this.callMethod('horizontalDimName',...args);}
  mergeDelimiters(...args: number[]): boolean {return this.callMethod('mergeDelimiters',...args);}
  missingValue(...args: number[]): number {return this.callMethod('missingValue',...args);}
  nColAxes(): number {return this.callMethod('nColAxes');}
  nRowAxes(): number {return this.callMethod('nRowAxes');}
  populateFromRavelMetadata(a1: string,a2: number): void {return this.callMethod('populateFromRavelMetadata',a1,a2);}
  quote(...args: number[]): number {return this.callMethod('quote',...args);}
  separator(...args: number[]): number {return this.callMethod('separator',...args);}
  setDataArea(a1: number,a2: number): void {return this.callMethod('setDataArea',a1,a2);}
  toSchema(): DataSpecSchema {return this.callMethod('toSchema');}
  toggleDimension(a1: number): void {return this.callMethod('toggleDimension',a1);}
}

export class EquationDisplay extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  item: Item;
  wire: Wire;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.item=new Item(this.prefix+'item');
    this.wire=new Wire(this.prefix+'wire');
  }
  controlMouseDown(...args: any[]): void {return this.callMethod('controlMouseDown',...args);}
  destroyFrame(...args: any[]): void {return this.callMethod('destroyFrame',...args);}
  disable(...args: any[]): void {return this.callMethod('disable',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getWireAt(...args: any[]): void {return this.callMethod('getWireAt',...args);}
  hasScrollBars(...args: any[]): boolean {return this.callMethod('hasScrollBars',...args);}
  height(...args: any[]): number {return this.callMethod('height',...args);}
  keyPress(...args: any[]): boolean {return this.callMethod('keyPress',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  position(...args: any[]): Sequence<number> {return this.callMethod('position',...args);}
  redraw(...args: any[]): boolean {return this.callMethod('redraw',...args);}
  redrawWithBounds(...args: any[]): void {return this.callMethod('redrawWithBounds',...args);}
  registerImage(...args: any[]): void {return this.callMethod('registerImage',...args);}
  renderFrame(...args: any[]): void {return this.callMethod('renderFrame',...args);}
  renderToEMF(...args: any[]): void {return this.callMethod('renderToEMF',...args);}
  renderToPDF(...args: any[]): void {return this.callMethod('renderToPDF',...args);}
  renderToPNG(...args: any[]): void {return this.callMethod('renderToPNG',...args);}
  renderToPS(...args: any[]): void {return this.callMethod('renderToPS',...args);}
  renderToSVG(...args: any[]): void {return this.callMethod('renderToSVG',...args);}
  reportDrawTime(...args: any[]): void {return this.callMethod('reportDrawTime',...args);}
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(...args: any[]): void {return this.callMethod('resizeWindow',...args);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  vectorRender(...args: any[]): ecolab__cairo__Surface {return this.callMethod('vectorRender',...args);}
  width(...args: any[]): number {return this.callMethod('width',...args);}
  zoom(...args: any[]): void {return this.callMethod('zoom',...args);}
}

export class EvalGodley extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  compatibility(...args: number[]): boolean {return this.callMethod('compatibility',...args);}
  eval(a1: number,a2: number): void {return this.callMethod('eval',a1,a2);}
}

export class FontDisplay extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
  }
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
}

export class GodleyIcon extends Item {
  bb: BoundingBox;
  editor: GodleyTableEditor;
  popup: GodleyTableWindow;
  svgRenderer: SVGRenderer;
  table: GodleyTable;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<GodleyIcon>prefix).prefix)
    this.bb=new BoundingBox(this.prefix+'bb');
    this.editor=new GodleyTableEditor(this.prefix+'editor');
    this.popup=new GodleyTableWindow(this.prefix+'popup');
    this.svgRenderer=new SVGRenderer(this.prefix+'svgRenderer');
    this.table=new GodleyTable(this.prefix+'table');
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(...args: any[]): void {return this.callMethod('TCL_obj',...args);}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustPopupWidgets(): void {return this.callMethod('adjustPopupWidgets');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bottom(): number {return this.callMethod('bottom');}
  bottomMargin(): number {return this.callMethod('bottomMargin');}
  buttonDisplay(): boolean {return this.callMethod('buttonDisplay');}
  checkUnits(): Units {return this.callMethod('checkUnits');}
  classType(...args: any[]): string {return this.callMethod('classType',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  clone(...args: any[]): Item {return this.callMethod('clone',...args);}
  closestInPort(a1: number,a2: number): Port {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): Port {return this.callMethod('closestOutPort',a1,a2);}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  corners(): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  deleteRow(a1: number): void {return this.callMethod('deleteRow',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: cairo_t,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  drawPorts(a1: cairo_t): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: cairo_t): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: cairo_t): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  editorMode(): boolean {return this.callMethod('editorMode');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  flip(): void {return this.callMethod('flip');}
  flowSignature(a1: number): Map<string,number> {return this.callMethod('flowSignature',a1);}
  flowVars(): Sequence<VariableBase> {return this.callMethod('flowVars');}
  height(): number {return this.callMethod('height');}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  inItem(...args: any[]): boolean {return this.callMethod('inItem',...args);}
  insertControlled(a1: Selection): void {return this.callMethod('insertControlled',a1);}
  ioVar(): boolean {return this.callMethod('ioVar');}
  itemPtrFromThis(): Item {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(...args: any[]): void {return this.callMethod('json_pack',...args);}
  left(): number {return this.callMethod('left');}
  leftMargin(): number {return this.callMethod('leftMargin');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveCell(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('moveCell',a1,a2,a3,a4);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(...args: any[]): boolean {return this.callMethod('onKeyPress',...args);}
  onMouseDown(...args: any[]): void {return this.callMethod('onMouseDown',...args);}
  onMouseLeave(...args: any[]): void {return this.callMethod('onMouseLeave',...args);}
  onMouseMotion(...args: any[]): boolean {return this.callMethod('onMouseMotion',...args);}
  onMouseOver(...args: any[]): boolean {return this.callMethod('onMouseOver',...args);}
  onMouseUp(...args: any[]): void {return this.callMethod('onMouseUp',...args);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): Port {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  removeControlledItems(a1: GroupItems): void {return this.callMethod('removeControlledItems',a1);}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  right(): number {return this.callMethod('right');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  rowSum(a1: number): string {return this.callMethod('rowSum',a1);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  scaleIcon(a1: number,a2: number): void {return this.callMethod('scaleIcon',a1,a2);}
  select(...args: any[]): Item {return this.callMethod('select',...args);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  setCell(a1: number,a2: number,a3: string): void {return this.callMethod('setCell',a1,a2,a3);}
  setCurrency(a1: string): void {return this.callMethod('setCurrency',a1);}
  stockVarUnits(a1: string,a2: boolean): Units {return this.callMethod('stockVarUnits',a1,a2);}
  stockVars(): Sequence<VariableBase> {return this.callMethod('stockVars');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  toggleButtons(): void {return this.callMethod('toggleButtons');}
  toggleEditorMode(): void {return this.callMethod('toggleEditorMode');}
  toggleVariableDisplay(): void {return this.callMethod('toggleVariableDisplay');}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  units(a1: boolean): Units {return this.callMethod('units',a1);}
  update(): void {return this.callMethod('update');}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  value(): number {return this.callMethod('value');}
  valueId(a1: string): string {return this.callMethod('valueId',a1);}
  variableDisplay(...args: number[]): boolean {return this.callMethod('variableDisplay',...args);}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.bottomRowMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.itemFocus=new Item(this.prefix+'itemFocus');
    this.itemVector=new Sequence<Item>(prefix,Item);
    this.rightColMargin=new Sequence<number>(prefix);
    this.varAttrib=new Sequence<string>(prefix);
    this.wire=new Wire(this.prefix+'wire');
  }
  cell(...args: any[]): ecolab__Pango {return this.callMethod('cell',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(...args: any[]): Item {return this.callMethod('itemAt',...args);}
  itemSelector(...args: any[]): boolean {return this.callMethod('itemSelector',...args);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  numCols(...args: any[]): number {return this.callMethod('numCols',...args);}
  numRows(...args: any[]): number {return this.callMethod('numRows',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): Sequence<number> {return this.callMethod('position');}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
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
  assetClass(a1: ecolab__TCL_args): string {return this.callMethod('assetClass',a1);}
  cell(...args: any[]): string {return this.callMethod('cell',...args);}
  cellInTable(a1: number,a2: number): boolean {return this.callMethod('cellInTable',a1,a2);}
  clear(): void {return this.callMethod('clear');}
  cols(): number {return this.callMethod('cols');}
  deleteCol(a1: number): void {return this.callMethod('deleteCol',a1);}
  deleteRow(a1: number): void {return this.callMethod('deleteRow',a1);}
  dimension(a1: number,a2: number): void {return this.callMethod('dimension',a1,a2);}
  doubleEntryCompliant(...args: number[]): boolean {return this.callMethod('doubleEntryCompliant',...args);}
  exportToCSV(a1: string): void {return this.callMethod('exportToCSV',a1);}
  exportToLaTeX(a1: string): void {return this.callMethod('exportToLaTeX',a1);}
  getCell(a1: number,a2: number): string {return this.callMethod('getCell',a1,a2);}
  getColumn(a1: number): Sequence<string> {return this.callMethod('getColumn',a1);}
  getColumnVariables(): Sequence<string> {return this.callMethod('getColumnVariables');}
  getData(): Sequence<Sequence<string>> {return this.callMethod('getData');}
  getVariables(): Sequence<string> {return this.callMethod('getVariables');}
  initialConditionRow(a1: number): boolean {return this.callMethod('initialConditionRow',a1);}
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
    this.colLeftMargin=new Sequence<number>(prefix);
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
  draw(a1: cairo_t): void {return this.callMethod('draw',a1);}
  drawButtons(...args: number[]): boolean {return this.callMethod('drawButtons',...args);}
  enableButtons(): void {return this.callMethod('enableButtons');}
  godleyIcon(...args: any[]): GodleyIcon {return this.callMethod('godleyIcon',...args);}
  height(): number {return this.callMethod('height');}
  highlightCell(a1: cairo_t,a2: number,a3: number): void {return this.callMethod('highlightCell',a1,a2,a3);}
  highlightColumn(a1: cairo_t,a2: number): void {return this.callMethod('highlightColumn',a1,a2);}
  highlightRow(a1: cairo_t,a2: number): void {return this.callMethod('highlightRow',a1,a2);}
  historyPtr(...args: number[]): number {return this.callMethod('historyPtr',...args);}
  hoverCol(...args: number[]): number {return this.callMethod('hoverCol',...args);}
  hoverRow(...args: number[]): number {return this.callMethod('hoverRow',...args);}
  importStockVar(a1: string,a2: number): void {return this.callMethod('importStockVar',a1,a2);}
  insertIdx(...args: number[]): number {return this.callMethod('insertIdx',...args);}
  keyPress(a1: number,a2: string): void {return this.callMethod('keyPress',a1,a2);}
  leftTableOffset(...args: number[]): number {return this.callMethod('leftTableOffset',...args);}
  matchingTableColumns(a1: number): Container<string> {return this.callMethod('matchingTableColumns',a1);}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.colLeftMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.wire=new Wire(this.prefix+'wire');
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
  drawButtons(...args: number[]): boolean {return this.callMethod('drawButtons',...args);}
  enableButtons(): void {return this.callMethod('enableButtons');}
  getItemAt(a1: number,a2: number): void {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  godleyIcon(...args: any[]): GodleyIcon {return this.callMethod('godleyIcon',...args);}
  hasScrollBars(...args: any[]): boolean {return this.callMethod('hasScrollBars',...args);}
  height(): number {return this.callMethod('height');}
  highlightCell(a1: cairo_t,a2: number,a3: number): void {return this.callMethod('highlightCell',a1,a2,a3);}
  highlightColumn(a1: cairo_t,a2: number): void {return this.callMethod('highlightColumn',a1,a2);}
  highlightRow(a1: cairo_t,a2: number): void {return this.callMethod('highlightRow',a1,a2);}
  historyPtr(...args: number[]): number {return this.callMethod('historyPtr',...args);}
  hoverCol(...args: number[]): number {return this.callMethod('hoverCol',...args);}
  hoverRow(...args: number[]): number {return this.callMethod('hoverRow',...args);}
  importStockVar(a1: string,a2: number): void {return this.callMethod('importStockVar',a1,a2);}
  insertIdx(...args: number[]): number {return this.callMethod('insertIdx',...args);}
  keyPress(...args: any[]): boolean {return this.callMethod('keyPress',...args);}
  leftTableOffset(...args: number[]): number {return this.callMethod('leftTableOffset',...args);}
  matchingTableColumns(a1: number): Container<string> {return this.callMethod('matchingTableColumns',a1);}
  maxHistory(...args: number[]): number {return this.callMethod('maxHistory',...args);}
  minColumnWidth(...args: number[]): number {return this.callMethod('minColumnWidth',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseMoveB1(a1: number,a2: number): void {return this.callMethod('mouseMoveB1',a1,a2);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveAssetClass(a1: number,a2: number): string {return this.callMethod('moveAssetClass',a1,a2);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  navigateDown(): void {return this.callMethod('navigateDown');}
  navigateLeft(): void {return this.callMethod('navigateLeft');}
  navigateRight(): void {return this.callMethod('navigateRight');}
  navigateUp(): void {return this.callMethod('navigateUp');}
  paste(): void {return this.callMethod('paste');}
  position(): Sequence<number> {return this.callMethod('position');}
  pulldownHot(...args: number[]): number {return this.callMethod('pulldownHot',...args);}
  pushHistory(): void {return this.callMethod('pushHistory');}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
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
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  zoom(...args: any[]): void {return this.callMethod('zoom',...args);}
  zoomFactor(...args: number[]): number {return this.callMethod('zoomFactor',...args);}
}

export class Group extends CppClass {
  bb: BoundingBox;
  bookmarks: Container<Bookmark>;
  createdIOvariables: Sequence<VariableBase>;
  groups: Sequence<Group>;
  inVariables: Sequence<VariableBase>;
  items: Sequence<Item>;
  outVariables: Sequence<VariableBase>;
  parent: Group;
  svgRenderer: SVGRenderer;
  wires: Sequence<Wire>;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.prefix+'bb');
    this.bookmarks=new Container<Bookmark>(prefix,Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.groups=new Sequence<Group>(prefix,Group);
    this.inVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.items=new Sequence<Item>(prefix,Item);
    this.outVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.parent=new Group(this.prefix+'parent');
    this.svgRenderer=new SVGRenderer(this.prefix+'svgRenderer');
    this.wires=new Sequence<Wire>(prefix,Wire);
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(...args: any[]): void {return this.callMethod('TCL_obj',...args);}
  accessibleVars(...args: any[]): Sequence<string> {return this.callMethod('accessibleVars',...args);}
  addBookmark(...args: any[]): void {return this.callMethod('addBookmark',...args);}
  addBookmarkXY(...args: any[]): void {return this.callMethod('addBookmarkXY',...args);}
  addGroup(...args: any[]): Group {return this.callMethod('addGroup',...args);}
  addInputVar(...args: any[]): void {return this.callMethod('addInputVar',...args);}
  addItem(...args: any[]): Item {return this.callMethod('addItem',...args);}
  addOutputVar(...args: any[]): void {return this.callMethod('addOutputVar',...args);}
  addWire(...args: any[]): Wire {return this.callMethod('addWire',...args);}
  adjustBookmark(...args: any[]): void {return this.callMethod('adjustBookmark',...args);}
  adjustWiresGroup(...args: any[]): void {return this.callMethod('adjustWiresGroup',...args);}
  arguments(...args: any[]): string {return this.callMethod('arguments',...args);}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  autoLayout(...args: any[]): void {return this.callMethod('autoLayout',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(...args: any[]): string {return this.callMethod('bookmarkId',...args);}
  bookmarkList(...args: any[]): Sequence<string> {return this.callMethod('bookmarkList',...args);}
  bottom(...args: any[]): number {return this.callMethod('bottom',...args);}
  cBounds(...args: any[]): Sequence<number> {return this.callMethod('cBounds',...args);}
  checkAddIORegion(...args: any[]): void {return this.callMethod('checkAddIORegion',...args);}
  checkUnits(...args: any[]): Units {return this.callMethod('checkUnits',...args);}
  classType(...args: any[]): string {return this.callMethod('classType',...args);}
  clear(...args: any[]): void {return this.callMethod('clear',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  clone(...args: any[]): Item {return this.callMethod('clone',...args);}
  closestInPort(...args: any[]): Port {return this.callMethod('closestInPort',...args);}
  closestOutPort(...args: any[]): Port {return this.callMethod('closestOutPort',...args);}
  computeDisplayZoom(...args: any[]): number {return this.callMethod('computeDisplayZoom',...args);}
  computeRelZoom(...args: any[]): void {return this.callMethod('computeRelZoom',...args);}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  contentBounds(...args: any[]): number {return this.callMethod('contentBounds',...args);}
  copy(...args: any[]): Group {return this.callMethod('copy',...args);}
  copyUnowned(...args: any[]): Group {return this.callMethod('copyUnowned',...args);}
  corners(...args: any[]): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners',...args);}
  defaultExtension(...args: any[]): string {return this.callMethod('defaultExtension',...args);}
  deleteAttachedWires(...args: any[]): void {return this.callMethod('deleteAttachedWires',...args);}
  deleteBookmark(...args: any[]): void {return this.callMethod('deleteBookmark',...args);}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  deleteItem(...args: any[]): void {return this.callMethod('deleteItem',...args);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(...args: any[]): void {return this.callMethod('disableDelayedTooltip',...args);}
  displayContents(...args: any[]): boolean {return this.callMethod('displayContents',...args);}
  displayContentsChanged(...args: any[]): boolean {return this.callMethod('displayContentsChanged',...args);}
  displayDelayedTooltip(...args: any[]): void {return this.callMethod('displayDelayedTooltip',...args);}
  displayTooltip(...args: any[]): void {return this.callMethod('displayTooltip',...args);}
  displayZoom(...args: number[]): number {return this.callMethod('displayZoom',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  draw1edge(...args: any[]): void {return this.callMethod('draw1edge',...args);}
  drawEdgeVariables(...args: any[]): void {return this.callMethod('drawEdgeVariables',...args);}
  drawIORegion(...args: any[]): void {return this.callMethod('drawIORegion',...args);}
  drawPorts(...args: any[]): void {return this.callMethod('drawPorts',...args);}
  drawResizeHandles(...args: any[]): void {return this.callMethod('drawResizeHandles',...args);}
  drawSelected(...args: any[]): void {return this.callMethod('drawSelected',...args);}
  dummyDraw(...args: any[]): void {return this.callMethod('dummyDraw',...args);}
  edgeScale(...args: any[]): number {return this.callMethod('edgeScale',...args);}
  empty(...args: any[]): boolean {return this.callMethod('empty',...args);}
  ensureBBValid(...args: any[]): void {return this.callMethod('ensureBBValid',...args);}
  findGroup(...args: any[]): Group {return this.callMethod('findGroup',...args);}
  findItem(...args: any[]): Item {return this.callMethod('findItem',...args);}
  findWire(...args: any[]): Wire {return this.callMethod('findWire',...args);}
  flip(...args: any[]): void {return this.callMethod('flip',...args);}
  flipContents(...args: any[]): void {return this.callMethod('flipContents',...args);}
  formula(...args: any[]): string {return this.callMethod('formula',...args);}
  globalGroup(...args: any[]): Group {return this.callMethod('globalGroup',...args);}
  gotoBookmark(...args: any[]): void {return this.callMethod('gotoBookmark',...args);}
  gotoBookmark_b(...args: any[]): void {return this.callMethod('gotoBookmark_b',...args);}
  height(...args: any[]): number {return this.callMethod('height',...args);}
  higher(...args: any[]): boolean {return this.callMethod('higher',...args);}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(...args: any[]): string {return this.callMethod('id',...args);}
  inIORegion(...args: any[]): string {return this.callMethod('inIORegion',...args);}
  inItem(...args: any[]): boolean {return this.callMethod('inItem',...args);}
  ioVar(...args: any[]): boolean {return this.callMethod('ioVar',...args);}
  itemPtrFromThis(...args: any[]): Item {return this.callMethod('itemPtrFromThis',...args);}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(...args: any[]): void {return this.callMethod('json_pack',...args);}
  left(...args: any[]): number {return this.callMethod('left',...args);}
  level(...args: any[]): number {return this.callMethod('level',...args);}
  localZoom(...args: any[]): number {return this.callMethod('localZoom',...args);}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  makeSubroutine(...args: any[]): void {return this.callMethod('makeSubroutine',...args);}
  margins(...args: any[]): void {return this.callMethod('margins',...args);}
  marginsV(...args: any[]): Sequence<number> {return this.callMethod('marginsV',...args);}
  minimalEnclosingGroup(...args: any[]): Group {return this.callMethod('minimalEnclosingGroup',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveContents(...args: any[]): void {return this.callMethod('moveContents',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  name(...args: any[]): string {return this.callMethod('name',...args);}
  nocycles(...args: any[]): boolean {return this.callMethod('nocycles',...args);}
  normaliseGroupRefs(...args: any[]): void {return this.callMethod('normaliseGroupRefs',...args);}
  numGroups(...args: any[]): number {return this.callMethod('numGroups',...args);}
  numItems(...args: any[]): number {return this.callMethod('numItems',...args);}
  numWires(...args: any[]): number {return this.callMethod('numWires',...args);}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(...args: any[]): boolean {return this.callMethod('onKeyPress',...args);}
  onMouseDown(...args: any[]): void {return this.callMethod('onMouseDown',...args);}
  onMouseLeave(...args: any[]): void {return this.callMethod('onMouseLeave',...args);}
  onMouseMotion(...args: any[]): boolean {return this.callMethod('onMouseMotion',...args);}
  onMouseOver(...args: any[]): boolean {return this.callMethod('onMouseOver',...args);}
  onMouseUp(...args: any[]): void {return this.callMethod('onMouseUp',...args);}
  onResizeHandle(...args: any[]): boolean {return this.callMethod('onResizeHandle',...args);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(...args: any[]): number {return this.callMethod('portX',...args);}
  portY(...args: any[]): number {return this.callMethod('portY',...args);}
  ports(...args: any[]): Port {return this.callMethod('ports',...args);}
  portsSize(...args: any[]): number {return this.callMethod('portsSize',...args);}
  px(...args: number[]): number {return this.callMethod('px',...args);}
  py(...args: number[]): number {return this.callMethod('py',...args);}
  pz(...args: number[]): number {return this.callMethod('pz',...args);}
  randomLayout(...args: any[]): void {return this.callMethod('randomLayout',...args);}
  relZoom(...args: number[]): number {return this.callMethod('relZoom',...args);}
  removeDisplayPlot(...args: any[]): void {return this.callMethod('removeDisplayPlot',...args);}
  removeGroup(...args: any[]): Group {return this.callMethod('removeGroup',...args);}
  removeItem(...args: any[]): Item {return this.callMethod('removeItem',...args);}
  removeWire(...args: any[]): Wire {return this.callMethod('removeWire',...args);}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleSize(...args: any[]): number {return this.callMethod('resizeHandleSize',...args);}
  resizeOnContents(...args: any[]): void {return this.callMethod('resizeOnContents',...args);}
  right(...args: any[]): number {return this.callMethod('right',...args);}
  rotFactor(...args: any[]): number {return this.callMethod('rotFactor',...args);}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(...args: any[]): Item {return this.callMethod('select',...args);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  setZoom(...args: any[]): void {return this.callMethod('setZoom',...args);}
  splitBoundaryCrossingWires(...args: any[]): void {return this.callMethod('splitBoundaryCrossingWires',...args);}
  throw_error(...args: any[]): void {return this.callMethod('throw_error',...args);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(...args: any[]): number {return this.callMethod('top',...args);}
  uniqueItems(...args: any[]): boolean {return this.callMethod('uniqueItems',...args);}
  units(...args: any[]): Units {return this.callMethod('units',...args);}
  updateBoundingBox(...args: any[]): void {return this.callMethod('updateBoundingBox',...args);}
  updateIcon(...args: any[]): void {return this.callMethod('updateIcon',...args);}
  updateTimestamp(): void {return this.callMethod('updateTimestamp');}
  value(...args: any[]): number {return this.callMethod('value',...args);}
  visible(...args: any[]): boolean {return this.callMethod('visible',...args);}
  visibleWithinGroup(...args: any[]): boolean {return this.callMethod('visibleWithinGroup',...args);}
  width(...args: any[]): number {return this.callMethod('width',...args);}
  x(...args: any[]): number {return this.callMethod('x',...args);}
  y(...args: any[]): number {return this.callMethod('y',...args);}
  zoom(...args: any[]): void {return this.callMethod('zoom',...args);}
  zoomFactor(...args: any[]): number {return this.callMethod('zoomFactor',...args);}
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
    this.bookmarks=new Container<Bookmark>(prefix,Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.groups=new Sequence<Group>(prefix,Group);
    this.inVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.items=new Sequence<Item>(prefix,Item);
    this.outVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.wires=new Sequence<Wire>(prefix,Wire);
  }
  addGroup(...args: any[]): Group {return this.callMethod('addGroup',...args);}
  addItem(...args: any[]): Item {return this.callMethod('addItem',...args);}
  addWire(...args: any[]): Wire {return this.callMethod('addWire',...args);}
  adjustWiresGroup(a1: Wire): void {return this.callMethod('adjustWiresGroup',a1);}
  clear(): void {return this.callMethod('clear');}
  empty(): boolean {return this.callMethod('empty');}
  findGroup(a1: Group): Group {return this.callMethod('findGroup',a1);}
  findItem(a1: Item): Item {return this.callMethod('findItem',a1);}
  findWire(a1: Wire): Wire {return this.callMethod('findWire',a1);}
  nocycles(): boolean {return this.callMethod('nocycles');}
  numGroups(): number {return this.callMethod('numGroups');}
  numItems(): number {return this.callMethod('numItems');}
  numWires(): number {return this.callMethod('numWires');}
  removeDisplayPlot(): void {return this.callMethod('removeDisplayPlot');}
  removeGroup(a1: Group): Group {return this.callMethod('removeGroup',a1);}
  removeItem(a1: Item): Item {return this.callMethod('removeItem',a1);}
  removeWire(a1: Wire): Wire {return this.callMethod('removeWire',a1);}
}

export class Item extends CppClass {
  bb: BoundingBox;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.prefix+'bb');
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(...args: any[]): void {return this.callMethod('TCL_obj',...args);}
  adjustBookmark(...args: any[]): void {return this.callMethod('adjustBookmark',...args);}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(...args: any[]): string {return this.callMethod('bookmarkId',...args);}
  bottom(...args: any[]): number {return this.callMethod('bottom',...args);}
  checkUnits(...args: any[]): Units {return this.callMethod('checkUnits',...args);}
  classType(...args: any[]): string {return this.callMethod('classType',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  clone(...args: any[]): Item {return this.callMethod('clone',...args);}
  closestInPort(...args: any[]): Port {return this.callMethod('closestInPort',...args);}
  closestOutPort(...args: any[]): Port {return this.callMethod('closestOutPort',...args);}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  corners(...args: any[]): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners',...args);}
  deleteAttachedWires(...args: any[]): void {return this.callMethod('deleteAttachedWires',...args);}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(...args: any[]): void {return this.callMethod('disableDelayedTooltip',...args);}
  displayDelayedTooltip(...args: any[]): void {return this.callMethod('displayDelayedTooltip',...args);}
  displayTooltip(...args: any[]): void {return this.callMethod('displayTooltip',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  drawPorts(...args: any[]): void {return this.callMethod('drawPorts',...args);}
  drawResizeHandles(...args: any[]): void {return this.callMethod('drawResizeHandles',...args);}
  drawSelected(...args: any[]): void {return this.callMethod('drawSelected',...args);}
  dummyDraw(...args: any[]): void {return this.callMethod('dummyDraw',...args);}
  ensureBBValid(...args: any[]): void {return this.callMethod('ensureBBValid',...args);}
  flip(...args: any[]): void {return this.callMethod('flip',...args);}
  height(...args: any[]): number {return this.callMethod('height',...args);}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(...args: any[]): string {return this.callMethod('id',...args);}
  inItem(...args: any[]): boolean {return this.callMethod('inItem',...args);}
  ioVar(...args: any[]): boolean {return this.callMethod('ioVar',...args);}
  itemPtrFromThis(...args: any[]): Item {return this.callMethod('itemPtrFromThis',...args);}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(...args: any[]): void {return this.callMethod('json_pack',...args);}
  left(...args: any[]): number {return this.callMethod('left',...args);}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(...args: any[]): boolean {return this.callMethod('onKeyPress',...args);}
  onMouseDown(...args: any[]): void {return this.callMethod('onMouseDown',...args);}
  onMouseLeave(...args: any[]): void {return this.callMethod('onMouseLeave',...args);}
  onMouseMotion(...args: any[]): boolean {return this.callMethod('onMouseMotion',...args);}
  onMouseOver(...args: any[]): boolean {return this.callMethod('onMouseOver',...args);}
  onMouseUp(...args: any[]): void {return this.callMethod('onMouseUp',...args);}
  onResizeHandle(...args: any[]): boolean {return this.callMethod('onResizeHandle',...args);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(...args: any[]): number {return this.callMethod('portX',...args);}
  portY(...args: any[]): number {return this.callMethod('portY',...args);}
  ports(...args: any[]): Port {return this.callMethod('ports',...args);}
  portsSize(...args: any[]): number {return this.callMethod('portsSize',...args);}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleSize(...args: any[]): number {return this.callMethod('resizeHandleSize',...args);}
  right(...args: any[]): number {return this.callMethod('right',...args);}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(...args: any[]): Item {return this.callMethod('select',...args);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  throw_error(...args: any[]): void {return this.callMethod('throw_error',...args);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(...args: any[]): number {return this.callMethod('top',...args);}
  units(...args: any[]): Units {return this.callMethod('units',...args);}
  updateBoundingBox(...args: any[]): void {return this.callMethod('updateBoundingBox',...args);}
  updateIcon(...args: any[]): void {return this.callMethod('updateIcon',...args);}
  value(...args: any[]): number {return this.callMethod('value',...args);}
  visible(...args: any[]): boolean {return this.callMethod('visible',...args);}
  visibleWithinGroup(...args: any[]): boolean {return this.callMethod('visibleWithinGroup',...args);}
  width(...args: any[]): number {return this.callMethod('width',...args);}
  x(...args: any[]): number {return this.callMethod('x',...args);}
  y(...args: any[]): number {return this.callMethod('y',...args);}
  zoomFactor(...args: any[]): number {return this.callMethod('zoomFactor',...args);}
}

export class LassoBox extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  angle(...args: number[]): number {return this.callMethod('angle',...args);}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  x0(...args: number[]): number {return this.callMethod('x0',...args);}
  x1(...args: number[]): number {return this.callMethod('x1',...args);}
  y0(...args: number[]): number {return this.callMethod('y0',...args);}
  y1(...args: number[]): number {return this.callMethod('y1',...args);}
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
    this.canvas=new Canvas(this.prefix+'canvas');
    this.conversions=new civita__Conversions(this.prefix+'conversions');
    this.dimensions=new Map<string,civita__Dimension>(prefix,civita__Dimension);
    this.equationDisplay=new EquationDisplay(this.prefix+'equationDisplay');
    this.evalGodley=new EvalGodley(this.prefix+'evalGodley');
    this.flowVars=new Sequence<number>(prefix);
    this.fontSampler=new FontDisplay(this.prefix+'fontSampler');
    this.godleyTab=new GodleyTab(this.prefix+'godleyTab');
    this.logVarList=new Container<string>(prefix);
    this.model=new Group(this.prefix+'model');
    this.namedItems=new Map<string,Item>(prefix,Item);
    this.panopticon=new Panopticon(this.prefix+'panopticon');
    this.parameterTab=new ParameterTab(this.prefix+'parameterTab');
    this.plotTab=new PlotTab(this.prefix+'plotTab');
    this.stockVars=new Sequence<number>(prefix);
    this.variableInstanceList=new VariableInstanceList(this.prefix+'variableInstanceList');
    this.variablePane=new VariablePane(this.prefix+'variablePane');
    this.variableTab=new VariableTab(this.prefix+'variableTab');
    this.variableValues=new VariableValues(this.prefix+'variableValues');
  }
  RKfunction(a1: number,a2: number,a3: number,a4: void): number {return this.callMethod('RKfunction',a1,a2,a3,a4);}
  addIntegral(): void {return this.callMethod('addIntegral');}
  assetClasses(): Sequence<string> {return this.callMethod('assetClasses');}
  autoLayout(): void {return this.callMethod('autoLayout');}
  autoSaveFile(): string {return this.callMethod('autoSaveFile');}
  availableOperations(): Sequence<string> {return this.callMethod('availableOperations');}
  balanceColumns(a1: GodleyIcon,a2: number,a3: GodleyIcon,a4: number): void {return this.callMethod('balanceColumns',a1,a2,a3,a4);}
  balanceDuplicateColumns(a1: GodleyIcon,a2: number): void {return this.callMethod('balanceDuplicateColumns',a1,a2);}
  checkEquationOrder(): boolean {return this.callMethod('checkEquationOrder');}
  checkMemAllocation(a1: number): boolean {return this.callMethod('checkMemAllocation',a1);}
  checkPushHistory(): void {return this.callMethod('checkPushHistory');}
  classifyOp(a1: string): string {return this.callMethod('classifyOp',a1);}
  clearAllMaps(...args: any[]): void {return this.callMethod('clearAllMaps',...args);}
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
  definingVar(a1: string): VariableBase {return this.callMethod('definingVar',a1);}
  deleteAllUnits(): void {return this.callMethod('deleteAllUnits');}
  deltaT(): number {return this.callMethod('deltaT');}
  dimensionalAnalysis(): void {return this.callMethod('dimensionalAnalysis');}
  displayErrorItem(a1: Item): void {return this.callMethod('displayErrorItem',a1);}
  displayStyle(...args: string[]): string {return this.callMethod('displayStyle',...args);}
  displayValues(...args: number[]): boolean {return this.callMethod('displayValues',...args);}
  doPushHistory(...args: number[]): boolean {return this.callMethod('doPushHistory',...args);}
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
  implicit(...args: number[]): boolean {return this.callMethod('implicit',...args);}
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
  listFonts(): Sequence<string> {return this.callMethod('listFonts');}
  load(a1: string): void {return this.callMethod('load',a1);}
  makeVariablesConsistent(): void {return this.callMethod('makeVariablesConsistent');}
  markEdited(): void {return this.callMethod('markEdited');}
  matchingTableColumns(a1: GodleyIcon,a2: string): Container<string> {return this.callMethod('matchingTableColumns',a1,a2);}
  matlab(a1: string): void {return this.callMethod('matlab',a1);}
  maxHistory(...args: number[]): number {return this.callMethod('maxHistory',...args);}
  maxWaitMS(...args: number[]): number {return this.callMethod('maxWaitMS',...args);}
  message(a1: string): void {return this.callMethod('message',a1);}
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
  renderAllPlotsAsSVG(a1: string): void {return this.callMethod('renderAllPlotsAsSVG',a1);}
  renderCanvasToEMF(a1: string): void {return this.callMethod('renderCanvasToEMF',a1);}
  renderCanvasToPDF(a1: string): void {return this.callMethod('renderCanvasToPDF',a1);}
  renderCanvasToPNG(a1: string): void {return this.callMethod('renderCanvasToPNG',a1);}
  renderCanvasToPS(a1: string): void {return this.callMethod('renderCanvasToPS',a1);}
  renderCanvasToSVG(a1: string): void {return this.callMethod('renderCanvasToSVG',a1);}
  renderEquationsToImage(a1: string): void {return this.callMethod('renderEquationsToImage',a1);}
  reset(): void {return this.callMethod('reset');}
  resetIfFlagged(...args: any[]): boolean {return this.callMethod('resetIfFlagged',...args);}
  reset_flag(): boolean {return this.callMethod('reset_flag');}
  reverse(...args: number[]): boolean {return this.callMethod('reverse',...args);}
  rkreset(): void {return this.callMethod('rkreset');}
  rkstep(): void {return this.callMethod('rkstep');}
  runItemDeletedCallback(a1: Item): void {return this.callMethod('runItemDeletedCallback',a1);}
  running(...args: number[]): boolean {return this.callMethod('running',...args);}
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
  step(): Sequence<number> {return this.callMethod('step');}
  stepMax(...args: number[]): number {return this.callMethod('stepMax',...args);}
  stepMin(...args: number[]): number {return this.callMethod('stepMin',...args);}
  t(...args: number[]): number {return this.callMethod('t',...args);}
  t0(...args: number[]): number {return this.callMethod('t0',...args);}
  timeUnit(...args: string[]): string {return this.callMethod('timeUnit',...args);}
  tmax(...args: number[]): number {return this.callMethod('tmax',...args);}
  triggerCheckMemAllocationCallback(): boolean {return this.callMethod('triggerCheckMemAllocationCallback');}
  undo(a1: number): number {return this.callMethod('undo',a1);}
  variableTypes(): Sequence<string> {return this.callMethod('variableTypes');}
}

export class Operation extends Item {
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<Operation>prefix).prefix)
  }
}

export class OperationBase extends CppClass {
  bb: BoundingBox;
  constructor(prefix: string){
    super(prefix);
    this.bb=new BoundingBox(this.prefix+'bb');
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  addPorts(): void {return this.callMethod('addPorts');}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  arg(...args: number[]): number {return this.callMethod('arg',...args);}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  axis(...args: string[]): string {return this.callMethod('axis',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bottom(): number {return this.callMethod('bottom');}
  checkUnits(): Units {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  classify(a1: string): string {return this.callMethod('classify',a1);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  clone(): Item {return this.callMethod('clone');}
  closestInPort(a1: number,a2: number): Port {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): Port {return this.callMethod('closestOutPort',a1,a2);}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  corners(): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners');}
  create(a1: string): OperationBase {return this.callMethod('create',a1);}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  dimensions(): Sequence<string> {return this.callMethod('dimensions');}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: cairo_t,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  drawPorts(a1: cairo_t): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(...args: any[]): void {return this.callMethod('drawResizeHandles',...args);}
  drawSelected(a1: cairo_t): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  flip(): void {return this.callMethod('flip');}
  h(...args: number[]): number {return this.callMethod('h',...args);}
  height(): number {return this.callMethod('height');}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  iconDraw(a1: cairo_t): void {return this.callMethod('iconDraw',a1);}
  id(): string {return this.callMethod('id');}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  ioVar(): boolean {return this.callMethod('ioVar');}
  itemPtrFromThis(): Item {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  l(...args: number[]): number {return this.callMethod('l',...args);}
  left(): number {return this.callMethod('left');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  multiWire(): boolean {return this.callMethod('multiWire');}
  numPorts(): number {return this.callMethod('numPorts');}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(...args: any[]): boolean {return this.callMethod('onResizeHandle',...args);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  operationCast(...args: any[]): OperationBase {return this.callMethod('operationCast',...args);}
  portValues(): string {return this.callMethod('portValues');}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): Port {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  r(...args: number[]): number {return this.callMethod('r',...args);}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleCoords(): boost__geometry__model__d2__point_xy {return this.callMethod('resizeHandleCoords');}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  right(): number {return this.callMethod('right');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): Item {return this.callMethod('select',a1,a2);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  units(...args: any[]): Units {return this.callMethod('units',...args);}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  value(...args: any[]): number {return this.callMethod('value',...args);}
  visible(): boolean {return this.callMethod('visible');}
  visibleWithinGroup(): boolean {return this.callMethod('visibleWithinGroup');}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoomFactor(): number {return this.callMethod('zoomFactor');}
}

export class Panopticon extends CppClass {
  backgroundColour: ecolab__cairo__Colour;
  constructor(prefix: string){
    super(prefix);
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
  }
  cheight(...args: number[]): number {return this.callMethod('cheight',...args);}
  cleft(...args: number[]): number {return this.callMethod('cleft',...args);}
  ctop(...args: number[]): number {return this.callMethod('ctop',...args);}
  cwidth(...args: number[]): number {return this.callMethod('cwidth',...args);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(): void {return this.callMethod('draw');}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(...args: number[]): number {return this.callMethod('height',...args);}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  width(...args: number[]): number {return this.callMethod('width',...args);}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.bottomRowMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.itemFocus=new Item(this.prefix+'itemFocus');
    this.itemVector=new Sequence<Item>(prefix,Item);
    this.rightColMargin=new Sequence<number>(prefix);
    this.varAttrib=new Sequence<string>(prefix);
    this.wire=new Wire(this.prefix+'wire');
  }
  cell(...args: any[]): ecolab__Pango {return this.callMethod('cell',...args);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): Item {return this.callMethod('itemAt',a1,a2);}
  itemSelector(...args: any[]): boolean {return this.callMethod('itemSelector',...args);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  numCols(...args: any[]): number {return this.callMethod('numCols',...args);}
  numRows(...args: any[]): number {return this.callMethod('numRows',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(...args: any[]): Sequence<number> {return this.callMethod('position',...args);}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.bottomRowMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.itemFocus=new Item(this.prefix+'itemFocus');
    this.itemVector=new Sequence<Item>(prefix,Item);
    this.rightColMargin=new Sequence<number>(prefix);
    this.varAttrib=new Sequence<string>(prefix);
    this.wire=new Wire(this.prefix+'wire');
  }
  cell(...args: any[]): ecolab__Pango {return this.callMethod('cell',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): Item {return this.callMethod('itemAt',a1,a2);}
  itemSelector(...args: any[]): boolean {return this.callMethod('itemSelector',...args);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  numCols(...args: any[]): number {return this.callMethod('numCols',...args);}
  numRows(...args: any[]): number {return this.callMethod('numRows',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(): Sequence<number> {return this.callMethod('position');}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  togglePlotDisplay(): void {return this.callMethod('togglePlotDisplay');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  width(): number {return this.callMethod('width');}
  xItem(...args: number[]): number {return this.callMethod('xItem',...args);}
  xoffs(...args: number[]): number {return this.callMethod('xoffs',...args);}
  yItem(...args: number[]): number {return this.callMethod('yItem',...args);}
  zoom(a1: number,a2: number,a3: number): void {return this.callMethod('zoom',a1,a2,a3);}
}

export class Port extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  checkUnits(): Units {return this.callMethod('checkUnits');}
  combineInput(a1: number,a2: number): void {return this.callMethod('combineInput',a1,a2);}
  deleteWires(): void {return this.callMethod('deleteWires');}
  eraseWire(a1: Wire): void {return this.callMethod('eraseWire',a1);}
  identity(): number {return this.callMethod('identity');}
  input(): boolean {return this.callMethod('input');}
  item(...args: any[]): Item {return this.callMethod('item',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  multiWireAllowed(): boolean {return this.callMethod('multiWireAllowed');}
  numWires(): number {return this.callMethod('numWires');}
  units(a1: boolean): Units {return this.callMethod('units',a1);}
  value(): number {return this.callMethod('value');}
  wires(): Sequence<Wire> {return this.callMethod('wires');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
}

export class SVGRenderer extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  height(...args: any[]): number {return this.callMethod('height',...args);}
  render(...args: any[]): void {return this.callMethod('render',...args);}
  setResource(...args: any[]): void {return this.callMethod('setResource',...args);}
  width(...args: any[]): number {return this.callMethod('width',...args);}
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
    this.bb=new BoundingBox(this.prefix+'bb');
    this.bookmarks=new Container<Bookmark>(prefix,Bookmark);
    this.createdIOvariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.groups=new Sequence<Group>(prefix,Group);
    this.inVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.items=new Sequence<Item>(prefix,Item);
    this.outVariables=new Sequence<VariableBase>(prefix,VariableBase);
    this.svgRenderer=new SVGRenderer(this.prefix+'svgRenderer');
    this.wires=new Sequence<Wire>(prefix,Wire);
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(...args: any[]): void {return this.callMethod('TCL_obj',...args);}
  accessibleVars(): Sequence<string> {return this.callMethod('accessibleVars');}
  addBookmark(a1: string): void {return this.callMethod('addBookmark',a1);}
  addBookmarkXY(a1: number,a2: number,a3: string): void {return this.callMethod('addBookmarkXY',a1,a2,a3);}
  addGroup(...args: any[]): Group {return this.callMethod('addGroup',...args);}
  addInputVar(): void {return this.callMethod('addInputVar');}
  addItem(...args: any[]): Item {return this.callMethod('addItem',...args);}
  addOutputVar(): void {return this.callMethod('addOutputVar');}
  addWire(...args: any[]): Wire {return this.callMethod('addWire',...args);}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustWiresGroup(a1: Wire): void {return this.callMethod('adjustWiresGroup',a1);}
  arguments(): string {return this.callMethod('arguments');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  autoLayout(): void {return this.callMethod('autoLayout');}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bookmarkList(): Sequence<string> {return this.callMethod('bookmarkList');}
  bottom(): number {return this.callMethod('bottom');}
  cBounds(): Sequence<number> {return this.callMethod('cBounds');}
  checkAddIORegion(a1: Item): void {return this.callMethod('checkAddIORegion',a1);}
  checkUnits(): Units {return this.callMethod('checkUnits');}
  classType(...args: any[]): string {return this.callMethod('classType',...args);}
  clear(...args: any[]): void {return this.callMethod('clear',...args);}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  clone(...args: any[]): Item {return this.callMethod('clone',...args);}
  closestInPort(a1: number,a2: number): Port {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): Port {return this.callMethod('closestOutPort',a1,a2);}
  computeDisplayZoom(): number {return this.callMethod('computeDisplayZoom');}
  computeRelZoom(): void {return this.callMethod('computeRelZoom');}
  contains(...args: any[]): boolean {return this.callMethod('contains',...args);}
  contentBounds(a1: number,a2: number,a3: number,a4: number): number {return this.callMethod('contentBounds',a1,a2,a3,a4);}
  copy(): Group {return this.callMethod('copy');}
  copyUnowned(): Group {return this.callMethod('copyUnowned');}
  corners(): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners');}
  defaultExtension(): string {return this.callMethod('defaultExtension');}
  deleteAttachedWires(...args: any[]): void {return this.callMethod('deleteAttachedWires',...args);}
  deleteBookmark(a1: number): void {return this.callMethod('deleteBookmark',a1);}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  deleteItem(a1: Item): void {return this.callMethod('deleteItem',a1);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayContents(): boolean {return this.callMethod('displayContents');}
  displayContentsChanged(): boolean {return this.callMethod('displayContentsChanged');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: cairo_t,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  displayZoom(...args: number[]): number {return this.callMethod('displayZoom',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  draw1edge(a1: Sequence<VariableBase>,a2: cairo_t,a3: number): void {return this.callMethod('draw1edge',a1,a2,a3);}
  drawEdgeVariables(a1: cairo_t): void {return this.callMethod('drawEdgeVariables',a1);}
  drawIORegion(a1: cairo_t): void {return this.callMethod('drawIORegion',a1);}
  drawPorts(a1: cairo_t): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(a1: cairo_t): void {return this.callMethod('drawResizeHandles',a1);}
  drawSelected(a1: cairo_t): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  edgeScale(): number {return this.callMethod('edgeScale');}
  empty(): boolean {return this.callMethod('empty');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  ensureGroupInserted(a1: Group): void {return this.callMethod('ensureGroupInserted',a1);}
  ensureItemInserted(a1: Item): void {return this.callMethod('ensureItemInserted',a1);}
  findGroup(a1: Group): Group {return this.callMethod('findGroup',a1);}
  findItem(a1: Item): Item {return this.callMethod('findItem',a1);}
  findWire(a1: Wire): Wire {return this.callMethod('findWire',a1);}
  flip(): void {return this.callMethod('flip');}
  flipContents(): void {return this.callMethod('flipContents');}
  formula(): string {return this.callMethod('formula');}
  globalGroup(...args: any[]): Group {return this.callMethod('globalGroup',...args);}
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
  itemPtrFromThis(): Item {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(...args: any[]): void {return this.callMethod('json_pack',...args);}
  left(): number {return this.callMethod('left');}
  level(): number {return this.callMethod('level');}
  localZoom(): number {return this.callMethod('localZoom');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  makeSubroutine(): void {return this.callMethod('makeSubroutine');}
  margins(a1: number,a2: number): void {return this.callMethod('margins',a1,a2);}
  marginsV(): Sequence<number> {return this.callMethod('marginsV');}
  minimalEnclosingGroup(...args: any[]): Group {return this.callMethod('minimalEnclosingGroup',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveContents(a1: Group): void {return this.callMethod('moveContents',a1);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  name(...args: any[]): string {return this.callMethod('name',...args);}
  nocycles(...args: any[]): boolean {return this.callMethod('nocycles',...args);}
  normaliseGroupRefs(a1: Group): void {return this.callMethod('normaliseGroupRefs',a1);}
  numGroups(): number {return this.callMethod('numGroups');}
  numItems(): number {return this.callMethod('numItems');}
  numWires(): number {return this.callMethod('numWires');}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(a1: number,a2: string,a3: number): boolean {return this.callMethod('onKeyPress',a1,a2,a3);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(a1: number,a2: number): boolean {return this.callMethod('onResizeHandle',a1,a2);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): Port {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  randomLayout(): void {return this.callMethod('randomLayout');}
  relZoom(...args: number[]): number {return this.callMethod('relZoom',...args);}
  removeDisplayPlot(): void {return this.callMethod('removeDisplayPlot');}
  removeGroup(a1: Group): Group {return this.callMethod('removeGroup',a1);}
  removeItem(a1: Item): Item {return this.callMethod('removeItem',a1);}
  removeWire(a1: Wire): Wire {return this.callMethod('removeWire',a1);}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  resizeOnContents(): void {return this.callMethod('resizeOnContents');}
  right(): number {return this.callMethod('right');}
  rotFactor(): number {return this.callMethod('rotFactor');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(...args: any[]): Item {return this.callMethod('select',...args);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  setZoom(a1: number): void {return this.callMethod('setZoom',a1);}
  splitBoundaryCrossingWires(): void {return this.callMethod('splitBoundaryCrossingWires');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  title(...args: string[]): string {return this.callMethod('title',...args);}
  toggleItemMembership(a1: Item): void {return this.callMethod('toggleItemMembership',a1);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  uniqueItems(...args: any[]): boolean {return this.callMethod('uniqueItems',...args);}
  units(a1: boolean): Units {return this.callMethod('units',a1);}
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

export class Units extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<Units>prefix).prefix)
  }
  latexStr(...args: any[]): string {return this.callMethod('latexStr',...args);}
  normalise(...args: any[]): void {return this.callMethod('normalise',...args);}
  str(...args: any[]): string {return this.callMethod('str',...args);}
}

export class VariableBase extends Item {
  bb: BoundingBox;
  constructor(prefix: string|Item){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<VariableBase>prefix).prefix)
    this.bb=new BoundingBox(this.prefix+'bb');
  }
  RESTProcess(...args: any[]): void {return this.callMethod('RESTProcess',...args);}
  TCL_obj(a1: classdesc__TCL_obj_t,a2: string): void {return this.callMethod('TCL_obj',a1,a2);}
  accessibleVars(): Sequence<string> {return this.callMethod('accessibleVars');}
  adjustBookmark(): void {return this.callMethod('adjustBookmark');}
  adjustSliderBounds(): void {return this.callMethod('adjustSliderBounds');}
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  bookmarkId(): string {return this.callMethod('bookmarkId');}
  bottom(): number {return this.callMethod('bottom');}
  checkUnits(): Units {return this.callMethod('checkUnits');}
  classType(): string {return this.callMethod('classType');}
  clickType(...args: any[]): string {return this.callMethod('clickType',...args);}
  clone(...args: any[]): Item {return this.callMethod('clone',...args);}
  closestInPort(a1: number,a2: number): Port {return this.callMethod('closestInPort',a1,a2);}
  closestOutPort(a1: number,a2: number): Port {return this.callMethod('closestOutPort',a1,a2);}
  contains(a1: number,a2: number): boolean {return this.callMethod('contains',a1,a2);}
  corners(): Sequence<boost__geometry__model__d2__point_xy> {return this.callMethod('corners');}
  create(a1: string): VariableBase {return this.callMethod('create',a1);}
  defined(): boolean {return this.callMethod('defined');}
  definition(): string {return this.callMethod('definition');}
  deleteAttachedWires(): void {return this.callMethod('deleteAttachedWires');}
  deleteCallback(...args: string[]): string {return this.callMethod('deleteCallback',...args);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  dimLabels(): Sequence<string> {return this.callMethod('dimLabels');}
  dims(): Sequence<number> {return this.callMethod('dims');}
  disableDelayedTooltip(): void {return this.callMethod('disableDelayedTooltip');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  displayTooltip(a1: cairo_t,a2: string): void {return this.callMethod('displayTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  drawPorts(a1: cairo_t): void {return this.callMethod('drawPorts',a1);}
  drawResizeHandles(...args: any[]): void {return this.callMethod('drawResizeHandles',...args);}
  drawSelected(a1: cairo_t): void {return this.callMethod('drawSelected',a1);}
  dummyDraw(): void {return this.callMethod('dummyDraw');}
  engExp(): EngNotation {return this.callMethod('engExp');}
  ensureBBValid(): void {return this.callMethod('ensureBBValid');}
  ensureValueExists(a1: VariableValue,a2: string): void {return this.callMethod('ensureValueExists',a1,a2);}
  exportAsCSV(a1: string): void {return this.callMethod('exportAsCSV',a1);}
  flip(): void {return this.callMethod('flip');}
  getDimLabelsPicked(): std__pair<std__string,std__string> {return this.callMethod('getDimLabelsPicked');}
  height(): number {return this.callMethod('height');}
  iHeight(...args: any[]): number {return this.callMethod('iHeight',...args);}
  iWidth(...args: any[]): number {return this.callMethod('iWidth',...args);}
  id(): string {return this.callMethod('id');}
  importFromCSV(a1: string,a2: DataSpecSchema): void {return this.callMethod('importFromCSV',a1,a2);}
  inItem(a1: number,a2: number): boolean {return this.callMethod('inItem',a1,a2);}
  init(...args: any[]): string {return this.callMethod('init',...args);}
  initSliderBounds(): void {return this.callMethod('initSliderBounds');}
  inputWired(): boolean {return this.callMethod('inputWired');}
  ioVar(...args: any[]): boolean {return this.callMethod('ioVar',...args);}
  isStock(): boolean {return this.callMethod('isStock');}
  itemPtrFromThis(): Item {return this.callMethod('itemPtrFromThis');}
  itemTabInitialised(...args: number[]): boolean {return this.callMethod('itemTabInitialised',...args);}
  itemTabX(...args: number[]): number {return this.callMethod('itemTabX',...args);}
  itemTabY(...args: number[]): number {return this.callMethod('itemTabY',...args);}
  json_pack(a1: classdesc__json_pack_t): void {return this.callMethod('json_pack',a1);}
  left(): number {return this.callMethod('left');}
  lhs(): boolean {return this.callMethod('lhs');}
  local(): boolean {return this.callMethod('local');}
  m_sf(...args: number[]): number {return this.callMethod('m_sf',...args);}
  m_x(...args: number[]): number {return this.callMethod('m_x',...args);}
  m_y(...args: number[]): number {return this.callMethod('m_y',...args);}
  mantissa(a1: EngNotation,a2: number): string {return this.callMethod('mantissa',a1,a2);}
  maxSliderSteps(): number {return this.callMethod('maxSliderSteps');}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  name(...args: any[]): string {return this.callMethod('name',...args);}
  numPorts(): number {return this.callMethod('numPorts');}
  onBorder(...args: number[]): boolean {return this.callMethod('onBorder',...args);}
  onKeyPress(...args: any[]): boolean {return this.callMethod('onKeyPress',...args);}
  onMouseDown(a1: number,a2: number): void {return this.callMethod('onMouseDown',a1,a2);}
  onMouseLeave(): void {return this.callMethod('onMouseLeave');}
  onMouseMotion(a1: number,a2: number): boolean {return this.callMethod('onMouseMotion',a1,a2);}
  onMouseOver(a1: number,a2: number): boolean {return this.callMethod('onMouseOver',a1,a2);}
  onMouseUp(a1: number,a2: number): void {return this.callMethod('onMouseUp',a1,a2);}
  onResizeHandle(...args: any[]): boolean {return this.callMethod('onResizeHandle',...args);}
  onResizeHandles(...args: number[]): boolean {return this.callMethod('onResizeHandles',...args);}
  portX(a1: number): number {return this.callMethod('portX',a1);}
  portY(a1: number): number {return this.callMethod('portY',a1);}
  ports(a1: number): Port {return this.callMethod('ports',a1);}
  portsSize(): number {return this.callMethod('portsSize');}
  rawName(): string {return this.callMethod('rawName');}
  resize(...args: any[]): void {return this.callMethod('resize',...args);}
  resizeHandleCoords(): boost__geometry__model__d2__point_xy {return this.callMethod('resizeHandleCoords');}
  resizeHandleSize(): number {return this.callMethod('resizeHandleSize');}
  retype(a1: string): void {return this.callMethod('retype',a1);}
  right(): number {return this.callMethod('right');}
  rotation(...args: any[]): number {return this.callMethod('rotation',...args);}
  scaleFactor(...args: any[]): number {return this.callMethod('scaleFactor',...args);}
  select(a1: number,a2: number): Item {return this.callMethod('select',a1,a2);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  setDimLabelsPicked(a1: string,a2: string): std__pair<std__string,std__string> {return this.callMethod('setDimLabelsPicked',a1,a2);}
  setUnits(a1: string): void {return this.callMethod('setUnits',a1);}
  sliderBoundsSet(...args: number[]): boolean {return this.callMethod('sliderBoundsSet',...args);}
  sliderMax(...args: number[]): number {return this.callMethod('sliderMax',...args);}
  sliderMin(...args: number[]): number {return this.callMethod('sliderMin',...args);}
  sliderSet(a1: number): void {return this.callMethod('sliderSet',a1);}
  sliderStep(...args: number[]): number {return this.callMethod('sliderStep',...args);}
  sliderStepRel(...args: number[]): boolean {return this.callMethod('sliderStepRel',...args);}
  temp(): boolean {return this.callMethod('temp');}
  throw_error(a1: string): void {return this.callMethod('throw_error',a1);}
  toggleLocal(): void {return this.callMethod('toggleLocal');}
  toggleVarTabDisplay(): void {return this.callMethod('toggleVarTabDisplay');}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  top(): number {return this.callMethod('top');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  units(...args: any[]): Units {return this.callMethod('units',...args);}
  unitsStr(): string {return this.callMethod('unitsStr');}
  updateBoundingBox(): void {return this.callMethod('updateBoundingBox');}
  updateIcon(a1: number): void {return this.callMethod('updateIcon',a1);}
  vValue(): VariableValue {return this.callMethod('vValue');}
  value(...args: any[]): number {return this.callMethod('value',...args);}
  valueId(): string {return this.callMethod('valueId');}
  valueIdInCurrentScope(a1: string): string {return this.callMethod('valueIdInCurrentScope',a1);}
  varTabDisplay(...args: number[]): boolean {return this.callMethod('varTabDisplay',...args);}
  variableCast(...args: any[]): VariableBase {return this.callMethod('variableCast',...args);}
  varsPassed(...args: number[]): number {return this.callMethod('varsPassed',...args);}
  visible(...args: any[]): boolean {return this.callMethod('visible',...args);}
  visibleWithinGroup(...args: any[]): boolean {return this.callMethod('visibleWithinGroup',...args);}
  width(): number {return this.callMethod('width');}
  x(): number {return this.callMethod('x');}
  y(): number {return this.callMethod('y');}
  zoomFactor(...args: any[]): number {return this.callMethod('zoomFactor',...args);}
}

export class VariableInstanceList extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  gotoInstance(a1: number): void {return this.callMethod('gotoInstance',a1);}
  names(): Sequence<string> {return this.callMethod('names');}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.bottomRowMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.rightColMargin=new Sequence<number>(prefix);
    this.selection=new Container<string>(prefix,string);
    this.wire=new Wire(this.prefix+'wire');
  }
  cell(...args: any[]): VariablePaneCell {return this.callMethod('cell',...args);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  deselect(a1: string): void {return this.callMethod('deselect',a1);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(...args: any[]): boolean {return this.callMethod('evenHeight',...args);}
  getItemAt(a1: number,a2: number): void {return this.callMethod('getItemAt',a1,a2);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(...args: any[]): boolean {return this.callMethod('hasScrollBars',...args);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mousePressed(...args: number[]): boolean {return this.callMethod('mousePressed',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(a1: number,a2: number): void {return this.callMethod('moveTo',a1,a2);}
  numCols(...args: any[]): number {return this.callMethod('numCols',...args);}
  numRows(...args: any[]): number {return this.callMethod('numRows',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  position(): Sequence<number> {return this.callMethod('position');}
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
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  select(a1: string): void {return this.callMethod('select',a1);}
  shift(...args: number[]): boolean {return this.callMethod('shift',...args);}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  update(): void {return this.callMethod('update');}
  updateWithHeight(a1: number): void {return this.callMethod('updateWithHeight',a1);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
  zoom(...args: any[]): void {return this.callMethod('zoom',...args);}
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
    this.backgroundColour=new ecolab__cairo__Colour(this.prefix+'backgroundColour');
    this.bottomRowMargin=new Sequence<number>(prefix);
    this.item=new Item(this.prefix+'item');
    this.itemFocus=new Item(this.prefix+'itemFocus');
    this.itemVector=new Sequence<Item>(prefix,Item);
    this.rightColMargin=new Sequence<number>(prefix);
    this.varAttrib=new Sequence<string>(prefix);
    this.wire=new Wire(this.prefix+'wire');
  }
  cell(...args: any[]): ecolab__Pango {return this.callMethod('cell',...args);}
  clickType(a1: number,a2: number): string {return this.callMethod('clickType',a1,a2);}
  colX(a1: number): number {return this.callMethod('colX',a1);}
  controlMouseDown(a1: number,a2: number): void {return this.callMethod('controlMouseDown',a1,a2);}
  destroyFrame(): void {return this.callMethod('destroyFrame');}
  disable(): void {return this.callMethod('disable');}
  displayDelayedTooltip(a1: number,a2: number): void {return this.callMethod('displayDelayedTooltip',a1,a2);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  evenHeight(): boolean {return this.callMethod('evenHeight');}
  getItemAt(...args: any[]): void {return this.callMethod('getItemAt',...args);}
  getVarName(a1: number): string {return this.callMethod('getVarName',a1);}
  getWireAt(a1: number,a2: number): void {return this.callMethod('getWireAt',a1,a2);}
  hasScrollBars(): boolean {return this.callMethod('hasScrollBars');}
  height(): number {return this.callMethod('height');}
  itemAt(a1: number,a2: number): Item {return this.callMethod('itemAt',a1,a2);}
  itemSelector(...args: any[]): boolean {return this.callMethod('itemSelector',...args);}
  justification(a1: number): string {return this.callMethod('justification',a1);}
  keyPress(a1: number,a2: string,a3: number,a4: number,a5: number): boolean {return this.callMethod('keyPress',a1,a2,a3,a4,a5);}
  m_height(...args: number[]): number {return this.callMethod('m_height',...args);}
  m_width(...args: number[]): number {return this.callMethod('m_width',...args);}
  mouseDown(...args: any[]): void {return this.callMethod('mouseDown',...args);}
  mouseMove(...args: any[]): void {return this.callMethod('mouseMove',...args);}
  mouseUp(...args: any[]): void {return this.callMethod('mouseUp',...args);}
  moveCursorTo(a1: number,a2: number): void {return this.callMethod('moveCursorTo',a1,a2);}
  moveItemTo(a1: number,a2: number): void {return this.callMethod('moveItemTo',a1,a2);}
  moveOffsX(...args: number[]): number {return this.callMethod('moveOffsX',...args);}
  moveOffsY(...args: number[]): number {return this.callMethod('moveOffsY',...args);}
  moveTo(...args: any[]): void {return this.callMethod('moveTo',...args);}
  numCols(...args: any[]): number {return this.callMethod('numCols',...args);}
  numRows(...args: any[]): number {return this.callMethod('numRows',...args);}
  offsx(...args: number[]): number {return this.callMethod('offsx',...args);}
  offsy(...args: number[]): number {return this.callMethod('offsy',...args);}
  populateItemVector(): void {return this.callMethod('populateItemVector');}
  position(...args: any[]): Sequence<number> {return this.callMethod('position',...args);}
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
  requestRedraw(...args: any[]): void {return this.callMethod('requestRedraw',...args);}
  resizeWindow(a1: number,a2: number,a3: number,a4: number): void {return this.callMethod('resizeWindow',a1,a2,a3,a4);}
  resolutionScaleFactor(...args: number[]): number {return this.callMethod('resolutionScaleFactor',...args);}
  rowY(a1: number): number {return this.callMethod('rowY',a1);}
  scaleFactor(): number {return this.callMethod('scaleFactor');}
  toggleVarDisplay(a1: number): void {return this.callMethod('toggleVarDisplay',a1);}
  vectorRender(a1: number,a2: __function__): ecolab__cairo__Surface {return this.callMethod('vectorRender',a1,a2);}
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
    this.csvDialog=new CSVDialog(this.prefix+'csvDialog');
    this.tensorInit=new civita__TensorVal(this.prefix+'tensorInit');
    this.units=new Units(this.prefix+'units');
  }
  allocValue(): VariableValue {return this.callMethod('allocValue');}
  atHCIndex(a1: number): number {return this.callMethod('atHCIndex',a1);}
  begin(...args: any[]): number {return this.callMethod('begin',...args);}
  end(...args: any[]): number {return this.callMethod('end',...args);}
  exportAsCSV(a1: string,a2: string): void {return this.callMethod('exportAsCSV',a1,a2);}
  godleyOverridden(...args: number[]): boolean {return this.callMethod('godleyOverridden',...args);}
  hypercube(...args: any[]): civita__Hypercube {return this.callMethod('hypercube',...args);}
  idx(): number {return this.callMethod('idx');}
  idxInRange(): boolean {return this.callMethod('idxInRange');}
  imposeDimensions(a1: Container<std__pair<const std__string,__civita__Dimension>>): void {return this.callMethod('imposeDimensions',a1);}
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
  shape(): Sequence<number> {return this.callMethod('shape');}
  size(): number {return this.callMethod('size');}
  sliderVisible(...args: number[]): boolean {return this.callMethod('sliderVisible',...args);}
  temp(): boolean {return this.callMethod('temp');}
  type(): string {return this.callMethod('type');}
  typeName(a1: number): string {return this.callMethod('typeName',a1);}
  unitsCached(...args: number[]): boolean {return this.callMethod('unitsCached',...args);}
  value(): number {return this.callMethod('value');}
  valueAt(a1: number): number {return this.callMethod('valueAt',a1);}
  valueId(): string {return this.callMethod('valueId');}
}

export class VariableValues extends Map<string,VariableValue> {
  constructor(prefix: string|Map<string,VariableValue>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<VariableValues>prefix).prefix)
  }
  clear(): void {return this.callMethod('clear');}
  initValue(...args: any[]): civita__TensorVal {return this.callMethod('initValue',...args);}
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
  attachedToDefiningVar(...args: any[]): boolean {return this.callMethod('attachedToDefiningVar',...args);}
  bookmark(...args: number[]): boolean {return this.callMethod('bookmark',...args);}
  coords(...args: any[]): Sequence<number> {return this.callMethod('coords',...args);}
  deleteHandle(...args: any[]): void {return this.callMethod('deleteHandle',...args);}
  detailedText(...args: string[]): string {return this.callMethod('detailedText',...args);}
  draw(...args: any[]): void {return this.callMethod('draw',...args);}
  editHandle(...args: any[]): void {return this.callMethod('editHandle',...args);}
  from(...args: any[]): Port {return this.callMethod('from',...args);}
  insertHandle(...args: any[]): void {return this.callMethod('insertHandle',...args);}
  mouseFocus(...args: number[]): boolean {return this.callMethod('mouseFocus',...args);}
  moveIntoGroup(...args: any[]): void {return this.callMethod('moveIntoGroup',...args);}
  moveToPorts(...args: any[]): void {return this.callMethod('moveToPorts',...args);}
  near(...args: any[]): boolean {return this.callMethod('near',...args);}
  nearestHandle(...args: any[]): number {return this.callMethod('nearestHandle',...args);}
  selected(...args: number[]): boolean {return this.callMethod('selected',...args);}
  split(...args: any[]): void {return this.callMethod('split',...args);}
  storeCairoCoords(...args: any[]): void {return this.callMethod('storeCairoCoords',...args);}
  straighten(...args: any[]): void {return this.callMethod('straighten',...args);}
  to(...args: any[]): Port {return this.callMethod('to',...args);}
  tooltip(...args: string[]): string {return this.callMethod('tooltip',...args);}
  units(...args: any[]): Units {return this.callMethod('units',...args);}
  visible(...args: any[]): boolean {return this.callMethod('visible',...args);}
}

export class civita__Conversions extends Map<string,number> {
  constructor(prefix: string|Map<string,number>){
    if (typeof prefix==='string')
      super(prefix)
    else
      super((<civita__Conversions>prefix).prefix)
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

export class civita__TensorVal extends CppClass {
  constructor(prefix: string){
    super(prefix);
  }
  allocVal(): void {return this.callMethod('allocVal');}
  atHCIndex(a1: number): number {return this.callMethod('atHCIndex',a1);}
  begin(...args: any[]): number {return this.callMethod('begin',...args);}
  end(...args: any[]): number {return this.callMethod('end',...args);}
  hypercube(...args: any[]): civita__Hypercube {return this.callMethod('hypercube',...args);}
  imposeDimensions(a1: Container<std__pair<const std__string,__civita__Dimension>>): void {return this.callMethod('imposeDimensions',a1);}
  index(...args: any[]): civita__Index {return this.callMethod('index',...args);}
  rank(): number {return this.callMethod('rank');}
  setArgument(a1: civita__ITensor,a2: string,a3: number): void {return this.callMethod('setArgument',a1,a2,a3);}
  setArguments(...args: any[]): void {return this.callMethod('setArguments',...args);}
  shape(): Sequence<number> {return this.callMethod('shape');}
  size(): number {return this.callMethod('size');}
  timestamp(): std__time_point {return this.callMethod('timestamp');}
  updateTimestamp(): void {return this.callMethod('updateTimestamp');}
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

export minsky: Minsky=new Minsky('minsky');
