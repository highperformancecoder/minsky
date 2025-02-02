import {
  CanvasItem,
  ClassType,
  events,
  Functions,
  HandleDimensionPayload,
  InitializePopupWindowPayload,
  InstallCase,
  HandleDescriptionPayload,
  importCSVvariableName,
  minsky, GodleyIcon, Group, IntOp, Item, Lock, Ravel, VariableBase, Wire, Utility, DownloadCSVPayload
} from '@minsky/shared';
import { app, dialog, ipcMain, Menu, MenuItem, SaveDialogOptions,} from 'electron';
import { existsSync, mkdirSync, renameSync, unlinkSync } from 'fs';
import { homedir } from 'os';
import JSON5 from 'json5';
import { extname, join, dirname } from 'path';
import { tmpdir } from 'os';
import { HelpFilesManager } from './HelpFilesManager';
import { WindowManager } from './WindowManager';
import { StoreManager } from './StoreManager';
import { RecentFilesManager } from './RecentFilesManager';
import ProgressBar from 'electron-progressbar';
import {exec,spawn} from 'child_process';
import decompress from 'decompress';
import {promisify} from 'util';

export class CommandsManager {
  static activeGodleyWindowItems = new Map<string, CanvasItem>();
  static currentMinskyModelFilePath: string;

  static async deleteCurrentItemHavingId(itemId: string) {
    // TODO:: Ideally -- change flow to get the current item here..
    // to ensure that we cannot mismatch itemId and currentItemId
    if (itemId) {
      WindowManager.closeWindowByUid(itemId);
      minsky.namedItems.erase(itemId.toString());
      minsky.canvas.deleteItem();
    }
  }

  private static async getItemClassType(
    x: number,
    y: number,
    raw = false
  ): Promise<ClassType | string> {
    minsky.canvas.getItemAt(x,y);
    return this.getCurrentItemClassType(raw);
  }

  private static async getCurrentItemClassType(
    raw = false
  ): Promise<ClassType | string> {
    const classTypeRes = await minsky.canvas.item.classType();

    if (typeof classTypeRes!=="string")
      return "";
    if (raw && classTypeRes) {
      return classTypeRes;
    }

    const classType = classTypeRes.includes(':')
      ? classTypeRes.split(':')[0]
      : classTypeRes;

    if (!classType) {
      return undefined;
    }
    return ClassType[classType];
  }

  static async getItemInfo(x: number, y: number): Promise<CanvasItem> {
    if (!await minsky.canvas.getItemAt(x, y)) {
      return null;
    }

    return await this.getCurrentItemInfo();
  }

  static async getFocusItemInfo(): Promise<CanvasItem> {
    await minsky.canvas.setItemFromItemFocus();
    return await this.getCurrentItemInfo();
  }

  static async getCurrentItemInfo() {
    const classType = (await this.getCurrentItemClassType()) as ClassType;
    const id = await minsky.canvas.item.id();
    
    if(!id) {
      return null;
    }

    let displayContents = false;
    if(classType === 'Group') {
      displayContents = await new Group(minsky.canvas.item).displayContents();
    }

    const itemInfo: CanvasItem = { classType, id, displayContents };
    return itemInfo;  
  }

  static async selectVar(x: number, y: number): Promise<boolean> {
    return minsky.canvas.selectVar(x,y);
  }

  static async flipSwitch(): Promise<void> {
    minsky.canvas.item.flip();
  }

  static async flip(): Promise<void> {
    minsky.canvas.item.flip();
    minsky.canvas.defaultRotation((await minsky.canvas.defaultRotation()+180)%360);
  }

  static async exportItemAsImageDialog(
    item: any,
    extension: string,
    name: string
  ): Promise<void> {
    const exportImage = await dialog.showSaveDialog({
      title: 'Export item as...',
      defaultPath: `export.${extension}`,
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [
        {name, extensions: [extension]},
        {name: 'All files', extensions: ['*']}
      ],
    });

    const { canceled, filePath} = exportImage;
    if (canceled || !filePath) {
      return;
    }
    this.exportItemAsImage(item,extension,filePath)
  }

  static exportItemAsImage(
    item: any,
    extension: string,
    filePath: string
  ) {
    switch (extension?.toLowerCase()) {
    case 'svg':
      item.renderToSVG(filePath);
      break;

    case 'pdf':
      item.renderToPDF(filePath);
      break;

    case 'ps': case 'eps':
      item.renderToPS(filePath);
      break;

    case 'emf':
      if (Functions.isWindows())
        item.renderToEMF(filePath);
      else
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: 'EMF only supported on MS Windows',
          type: 'error',
        });
      break;
    case 'png':
      item.renderToPNG(filePath);
      break;
    default:
      break;
    }
  }

  static async exportItemAsCSV(item: any, tabular=false): Promise<void> {
    const exportItemDialog = await dialog.showSaveDialog({
      title: 'Export item as csv',
      defaultPath: 'item.csv',
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [
        { extensions: ['csv'], name: 'CSV' },
        { extensions: ['*'], name: 'ALL' },
      ],
    });

    const { canceled, filePath} = exportItemDialog;

    if (canceled || !filePath) {
      return;
    }

    item.exportAsCSV(filePath, tabular);
    return;
  }

  static async renameAllInstances(itemInfo: CanvasItem): Promise<void> {
    switch (itemInfo.classType) {
      case ClassType.Variable:
      case ClassType.VarConstant:
        CommandsManager.openRenameInstancesDialog(
          await new VariableBase(minsky.canvas.item).name()
        );
        break;

      case ClassType.IntOp:
      case ClassType.DataOp:
        CommandsManager.openRenameInstancesDialog(
          await new IntOp(minsky.canvas.item).description()
        );
        break;

      default:
        break;
    }
  }

  private static openRenameInstancesDialog(name: string) {
    WindowManager.createPopupWindowWithRouting({
      title: `Rename ${name}`,
      url: `#/headless/rename-all-instances?name=${encodeURIComponent(name) || ''}`,
      height: 100,
      width: 400,
    });
  }

  static async editGodleyTitle(godley: GodleyIcon): Promise<void> {
    let title = await godley.table.title();
    let godleyId = await godley.id();

    minsky.nameCurrentItem(godleyId); // name current item
    WindowManager.createPopupWindowWithRouting({
      title: `Edit godley title`,
      url: `#/headless/edit-godley-title?title=${encodeURIComponent(title) || ''}&itemId=${godleyId}`,
      useContentSize: true,
      height: 100,
      width: 400,
    });
  }

  static async setGodleyCurrency(): Promise<void> {
    WindowManager.createPopupWindowWithRouting({
      title: `Edit godley currency`,
      url: `#/headless/edit-godley-currency`,
      useContentSize: true,
      height: 100,
      width: 400,
    });
  }

  static async postNote(type: string) {
    var item: Item|Wire;
    switch (type) {
    case 'item':
      item=minsky.canvas.item;
      break;
    case 'wire':
      item=minsky.canvas.wire;
      break;
    }

    const window=WindowManager.createPopupWindowWithRouting({
      title: `Description`,
      url: `#/headless/edit-description?type=${type}&bookmark=${await item.bookmark()}&tooltip=${encodeURIComponent(await item.tooltip())}&detailedText=${encodeURIComponent(await item.detailedText())}`,
    });
    Object.defineProperty(window,'dontCloseOnReturn',{value: true,writable:false});
  }

  static async getItemDims(
    x: number = null,
    y: number = null,
    reInvokeGetItemAt = false
  ): Promise<number[]> {
    try {
      if (reInvokeGetItemAt) {
        if (!x && !y) {
          throw new Error('Please provide x and y when reInvokeGetItemAt=true');
        }
        minsky.canvas.getItemAt(x,y);
      }

      return new VariableBase(minsky.canvas.item).dims();
    } catch (error) {
      console.error(
        '🚀 ~ file: commandsManager.ts ~ line 361 ~ CommandsManager ~ error',
        error
      );
      return null;
    }
  }

  static bookmarkThisPosition(): void {
    WindowManager.createPopupWindowWithRouting({
      width: 420,
      height: 250,
      title: 'Bookmarks',
      url: `#/headless/menu/bookmarks/add-bookmark`,
    });

    return;
  }

  static async getModelX(): Promise<number> {
    return minsky.model.x();
  }

  static async getModelY(): Promise<number> {
    return minsky.model.y();
  }

  static async bookmarkAt(x: number, y: number): Promise<void> {
    //  centre x,y in the visible canvas

    const modelX = await this.getModelX();
    const modelY = await this.getModelY();

    const delX = 0.5 * WindowManager.canvasWidth - x + modelX;
    const delY = 0.5 * WindowManager.canvasHeight - y + modelY;

    minsky.canvas.moveTo(delX,delY);
    this.bookmarkThisPosition();
    return;
  }

  static async pasteAt(x: number, y: number): Promise<void> {
    minsky.paste();
    WindowManager.currentTab.mouseMove(x,y);
  }

  static async saveSelectionAsFile(): Promise<void> {
    const saveDialog = await dialog.showSaveDialog({
      defaultPath: 'selection.mky',
    });

    const { canceled, filePath } = saveDialog;

    if (canceled || !filePath) {
      return;
    }

    minsky.saveSelectionAsFile(filePath);
  }

  static async findDefinition(): Promise<void> {
    
    const findVariableDefinition = await minsky.canvas.findVariableDefinition();

    if (findVariableDefinition) {
      const itemX = await minsky.model.x();
      const itemY = await minsky.model.y();
      const { canvasHeight, canvasWidth } = WindowManager;

      if (
        Math.abs(itemX - 0.5 * canvasWidth) > 0.5 * canvasWidth ||
        Math.abs(itemY - 0.5 * canvasHeight) > 0.5 * canvasHeight
      ) {
        const posX = itemX - (await minsky.canvas.itemIndicator.x()) + 0.5 * canvasWidth;
        const posY = itemY - (await minsky.canvas.itemIndicator.y()) + 0.5 * canvasHeight;
        minsky.canvas.moveTo(posX,posY);
      }
      minsky.canvas.requestRedraw();
      WindowManager.getMainWindow()?.webContents?.send(events.RESET_SCROLL);
    } else {
      dialog.showMessageBoxSync(WindowManager.getMainWindow(), {
        type: 'info',
        message: 'Definition not found',
      });
    }

    return;
  }

  static async getFilePathUsingSaveDialog(): Promise<string> {
    const saveDialog = await dialog.showSaveDialog({});

    const { canceled, filePath} = saveDialog;

    if (canceled || !filePath) {
      return null;
    }

    return filePath;
  }

  static async getFilePathFromExportCanvasDialog(
    type: string, label: string
  ): Promise<string> {
    const exportCanvasDialog = await dialog.showSaveDialog({
      title: 'Export canvas',
      defaultPath: `canvas.${type}`,
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [
        {name: label, extensions: [type]},
        {name: 'All files', extensions: ['*']}
      ],
    });

    let { canceled, filePath } = exportCanvasDialog;
    if (canceled) {
      return null;
    }
    
    // add extension if not already provided
    if (!filePath.includes('.'))
      filePath+=`.${type}`;

    return filePath;
  }

  static async mouseDown(mouseX: number, mouseY: number): Promise<void> {
    WindowManager.currentTab?.mouseDown(mouseX, mouseY);
  }

  static async mouseUp(mouseX: number, mouseY: number): Promise<void> {
    WindowManager.currentTab?.mouseUp(mouseX, mouseY);
  }

  static async mouseMove(mouseX: number, mouseY: number): Promise<void> {
    WindowManager.currentTab?.mouseMove(mouseX, mouseY);
  }

  static async requestRedraw(): Promise<void> {
    WindowManager.currentTab?.requestRedraw();
  }

  static async canCurrentSystemBeClosed(): Promise<boolean> {
    if (!await minsky.edited()) {
      return true;
    }

    const choice = await dialog.showMessageBox(WindowManager.getMainWindow(), {
      type: 'question',
      buttons: ['Yes', 'No', 'Cancel'],
      title: 'Confirm',
      message: 'Save Current Model?',
    });

    if (choice.response === 1) {
      // No
      return true;
    }
    if (choice.response === 2) {
      // Cancel
      return false;
    }

    await CommandsManager.save();
    return true;
  }

  static undo(changes: number) {
    WindowManager.activeWindows.forEach((window) => {
      if (!window.isMainWindow && window.context.title!=='Variables') {
        window.context.close();
      }
    });
    minsky.undo(changes);
    if (WindowManager.currentTab?.$prefix().includes("publicationTabs"))
      WindowManager.renderFrame(); // required because undo rewrites all the pubtabs
  }

  // clears the system to an empty state
  // return true if successful, false if user cancelled.
  static async createNewSystem() {
    if (!await this.canCurrentSystemBeClosed()) {
      return false;
    }

    WindowManager.activeWindows.forEach((window) => {
      if (!window.isMainWindow) {
        window.context.close();
      }
    });

    WindowManager.getMainWindow().setTitle('New System');
    this.currentMinskyModelFilePath="";
    
    minsky.doPushHistory(false);
    minsky.clearAllMaps();
    minsky.pushFlags();
    minsky.clearHistory();
    minsky.model.setZoom(1);
    minsky.canvas.recentre();
    minsky.popFlags();
    await minsky.doPushHistory(true);
    WindowManager.getMainWindow()?.webContents?.send(events.CHANGE_MAIN_TAB); // not necesarily removed, maybe added
    WindowManager.getMainWindow()?.webContents?.send(events.PUB_TAB_REMOVED); // not necesarily removed, maybe added
    return true;
  }

  static async saveGroupAsFile(): Promise<void> {
    const defaultExtension = await minsky.model.defaultExtension();

    const saveDialog = await dialog.showSaveDialog({
      filters: [
        {
          name: defaultExtension,
          extensions: [defaultExtension.slice(1)],
        },
        { name: 'All', extensions: ['*'] },
      ],
      defaultPath: `group${defaultExtension}`,
      properties: ['showOverwriteConfirmation'],
    });

    const { canceled, filePath} = saveDialog;

    if (canceled || !filePath) {
      return;
    }

    minsky.saveCanvasItemAsFile(filePath);
  }

  static async exportGodleyAs(
    ext: string,
    command: (x: string)=>void = null
  ): Promise<void> {
    const saveDialog = await dialog.showSaveDialog({
      filters: [
        {
          name: '.' + ext,
          extensions: [ext],
        },
        { name: 'All', extensions: ['*'] },
      ],
      defaultPath: `godley.${ext}`,
      properties: ['showOverwriteConfirmation'],
    });

    const { canceled, filePath } = saveDialog;

    if (canceled || !filePath) {
      return;
    }

    if (command) {
      command(filePath);
      return;
    }

    const godley=new GodleyIcon(minsky.canvas.item);
    switch (ext) {
    case 'csv':
      godley.table.exportToCSV(filePath);
      break;

    case 'tex':
      godley.table.exportToLaTeX(filePath);
      break;

      default:
        break;
    }

    return;
  }

  static async openNamedFile(filePath: string) {
    const autoBackupFileName = filePath + '#';

    if (!await this.createNewSystem()) return;

    WindowManager.scrollToCenter();

    const autoBackupFileExists = existsSync(autoBackupFileName);

    if (!autoBackupFileExists) {
      minsky.load(filePath);
      ipcMain.emit(events.ADD_RECENT_FILE, null, filePath);
    } else {
      const choice = dialog.showMessageBoxSync(WindowManager.getMainWindow(), {
        type: 'question',
        buttons: ['Yes', 'No'],
        title: 'Confirm',
        message: 'Auto save file exists, do you wish to load it?',
      });

      if (choice === 0) {
        minsky.load(autoBackupFileName);
      } else {
        minsky.load(filePath);
        ipcMain.emit(events.ADD_RECENT_FILE, null, filePath);
        unlinkSync(autoBackupFileName);
      }
    }

    minsky.setAutoSaveFile(autoBackupFileName);

    this.currentMinskyModelFilePath = filePath;

    setTimeout(()=>{minsky.canvas.recentre();},100);

    WindowManager.getMainWindow().setTitle(filePath);
    WindowManager.getMainWindow()?.webContents?.send(events.PUB_TAB_REMOVED); // not necesarily removed, may have been added
  }

  static async help(x: number, y: number) {
    let classType = (await this.getItemClassType(x, y, true)) as string;

    if (!classType) {
      classType = (await minsky.canvas.getWireAt(x,y)) ? 'Wires' : 'DesignCanvas';
    }

    this.loadHelpFile(classType);
    return;
  }

  static async loadHelpFile(classType: string) {
    const fileName = HelpFilesManager.getHelpFileForType(classType);

    const path = !Utility.isPackaged()
      ? `${join(__dirname, '../../../', `minsky-docs/${fileName}`)}`
      : `${join(process.resourcesPath, `minsky-docs/${fileName}`)}`;

    WindowManager.createMenuPopUpAndLoadFile({
      title: `Help: ${classType}`,
      height: 600,
      width: 1000,
      modal: false,
      url: path,
    });
  }

  static async findAllInstances() {
    minsky.listAllInstances();
    const instances = await minsky.variableInstanceList.names();

    if (!instances.length) {
      return;
    }

    WindowManager.createPopupWindowWithRouting({
      title: `Instances`,
      height: 500,
      width: 300,
      modal: false,
      url: `#/headless/find-all-instances`,
    });
  }

  static async editVar() {
    const v=new VariableBase(minsky.canvas.item);
    const name=await v.name();
    const type=await v.type();
    const local=await v.local();
    WindowManager.createPopupWindowWithRouting({
      width: 400,
      height: 500,
      title: `Edit ${name || ''}`,
      url: `#/headless/menu/insert/create-variable?type=${type}&name=${encodeURIComponent(name)||''}&isEditMode=true&local=${local}`,
    });
  }

  static async editItem(classType: string) {
    let height;
    let width = 500;
    switch (classType) {
      case ClassType.Group:
        width = 400;
        height = 130;
        break;
      case ClassType.Operation:
        height = 200;
        width = 300;
        break;
      case ClassType.UserFunction:
        height = 250;
        width = 350;
        break;
      default:
        height = 410;
        break;
    }
    WindowManager.createPopupWindowWithRouting({
      width: width,
      height,
      title: `Edit ${classType || ''}`,
      url: `#/headless/edit-${classType.toLowerCase()}`,
    });
  }

  private static onPopupWindowClose(uid: string) {
    minsky.namedItems.elem(uid).destroyFrame();
    if (uid in this.activeGodleyWindowItems) {
      this.activeGodleyWindowItems.delete(uid);
    }
  }

  private static async initializePopupWindow(
    payload: InitializePopupWindowPayload
  ): Promise<Electron.BrowserWindow> {
    const { itemInfo, url, height = 600, width = 800, minHeight, minWidth, modal = true } = payload;
    await CommandsManager.addItemToNamedItems(itemInfo);

    const window = WindowManager.createPopupWindowWithRouting(
      {
        title: payload.customTitle
          ? payload.customTitle
          : itemInfo.classType + ' : ' + itemInfo.id,
        url: url,
        uid: itemInfo.id,
        height,
        width,
        minHeight,
        minWidth,
        modal,
      },
      () => {
        this.onPopupWindowClose(itemInfo.id);
      }
    );
    return window;
  }

  static async addItemToNamedItems(itemInfo: CanvasItem) {
    const idString = itemInfo.id.toString();
    if(idString.includes('object')) {
      throw new Error(`Invalid ID value on itemInfo ${JSON5.stringify(itemInfo)}`);
    }
    // Pushing the current item to namedItems map
    minsky.nameCurrentItem(itemInfo.id.toString());
  }

  static async handleDoubleClick({ mouseX, mouseY }) {
    const itemInfo = await CommandsManager.getItemInfo(mouseX, mouseY);
    // TODO:: When opening a new popup for plot / godley or closing it,
    // notify the backend

    if (itemInfo?.classType) {
      switch (itemInfo?.classType) {
        case ClassType.GodleyIcon:
          await CommandsManager.openGodleyTable(itemInfo);
          break;

        case ClassType.PlotWidget:
          await CommandsManager.expandPlot(itemInfo);
        break;
        
        case ClassType.Ravel:
          await CommandsManager.openRavelPopup(itemInfo);
          break;

        case ClassType.Variable:
        case ClassType.VarConstant:
          await CommandsManager.editVar();
          break;

        case ClassType.Operation:
          await CommandsManager.editItem(ClassType.Operation);

          break;

        case ClassType.IntOp:
        case ClassType.DataOp:
          await CommandsManager.editItem(ClassType.IntOp);

          break;

        case ClassType.UserFunction:
          await CommandsManager.editItem(ClassType.UserFunction);

          break;

      case ClassType.Group:
        if (await CommandsManager.selectVar(mouseX,mouseY))
          await CommandsManager.editVar();
        else
          await CommandsManager.editItem(ClassType.Group);
        break;

        case ClassType.Item:
          await CommandsManager.postNote('item');
          break;

      case ClassType.Lock:
        new Lock(minsky.canvas.item).toggleLocked();
        minsky.canvas.requestRedraw();
        break;
        
        default:
          break;
      }
    }
  }

  static async openGodleyTable(itemInfo: CanvasItem) {
    if (!WindowManager.focusIfWindowIsPresent(itemInfo.id)) {
      CommandsManager.addItemToNamedItems(itemInfo);
      let godley=new GodleyIcon(minsky.namedItems.elem(itemInfo.id));
      var title=await godley.table.title();

      const window = await this.initializePopupWindow({
        customTitle: `Godley Table : ${title}`,
        itemInfo,
        url: `#/headless/godley-widget-view?systemWindowId=0&itemId=${itemInfo.id}`,
        modal: false,
      });

      Object.defineProperty(window,'dontCloseOnEscape',{value: true,writable:false});
      godley.adjustPopupWidgets();

      let systemWindowId = WindowManager.getWindowByUid(itemInfo.id).systemWindowId;
      
      window.loadURL(
        WindowManager.getWindowUrl(
          `#/headless/godley-widget-view?systemWindowId=${systemWindowId}&itemId=${itemInfo.id}`
        )
      );
      
      ipcMain.emit(events.INIT_MENU_FOR_GODLEY_VIEW, null, {
        window,
        itemInfo,
      });
      
      this.activeGodleyWindowItems.set(itemInfo.id, itemInfo);
    }
  }

  static async expandPlot(itemInfo: CanvasItem) {
    if (!WindowManager.focusIfWindowIsPresent(itemInfo.id)) {
      CommandsManager.addItemToNamedItems(itemInfo);
      const window = await this.initializePopupWindow({
        customTitle: `Plot : ${itemInfo.id}`,
        itemInfo,
        url: `#/headless/plot-widget-view?systemWindowId=0&itemId=${itemInfo.id}`,
        modal: false,
      });

      let systemWindowId = WindowManager.getWindowByUid(itemInfo.id).systemWindowId;

      window.loadURL(
        WindowManager.getWindowUrl(
          `#/headless/plot-widget-view?systemWindowId=${systemWindowId}&itemId=${itemInfo.id}`
        )
      );
      const menu = Menu.buildFromTemplate([
        new MenuItem({
          label: 'Options',
          submenu: [
            {
              label: 'Options',
              click: async () => {
                await CommandsManager.openPlotWindowOptions(itemInfo);
              },
            },
          ],
        }),
      ]);
      WindowManager.storeWindowMenu(window, menu);
      if (Functions.isMacOS()) {
        Menu.setApplicationMenu(menu);
      } else {
        window.setMenu(menu);
      }
    }
  }

  static async openRavelPopup(itemInfo: CanvasItem) {
    if (!WindowManager.focusIfWindowIsPresent(itemInfo.id)) {
      CommandsManager.addItemToNamedItems(itemInfo);
      const window = await this.initializePopupWindow({
        customTitle: `Ravel : ${itemInfo.id}`,
        itemInfo,
        url: `#/headless/ravel-widget-view?systemWindowId=0&itemId=${itemInfo.id}`,
        modal: false,
      });

      let systemWindowId = WindowManager.getWindowByUid(itemInfo.id).systemWindowId;

      window.loadURL(
        WindowManager.getWindowUrl(
          `#/headless/ravel-widget-view?systemWindowId=${systemWindowId}&itemId=${itemInfo.id}`
        )
      );
    }
  }

  static async openPlotWindowOptions(itemInfo: CanvasItem) {
    await CommandsManager.addItemToNamedItems(itemInfo);
    WindowManager.createPopupWindowWithRouting({
      title: 'Plot Window Options',
      url: `#/headless/plot-widget-options?itemId=${itemInfo.id}`,
      uid: itemInfo.id,
      height: 700,
      width: 500,
    });
  }

  static setLogSimulationCheckmark(checked: boolean) {
    Menu.getApplicationMenu().getMenuItemById('logging-menu-item').checked=checked;
  }
  
  static async logSimulation(selectedItems: string[]) {
    if (!Array.isArray(selectedItems) || !selectedItems.length) {
      this.setLogSimulationCheckmark(false);
      return;
    }

    const logSimulation = await dialog.showSaveDialog({
      title: 'Save As',
      defaultPath: 'log_simulation.csv',
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [{ extensions: ['csv'], name: 'CSV' }],
    });

    const { canceled, filePath} = logSimulation;

    if (canceled || !filePath) {
      this.setLogSimulationCheckmark(false);
      return;
    }

    minsky.logVarList.properties(selectedItems);
    minsky.openLogFile(filePath);
    this.setLogSimulationCheckmark(true);
  }

  static async importCSV(itemInfo: CanvasItem, isInvokedUsingToolbar = false) {
    if (!WindowManager.focusIfWindowIsPresent(itemInfo.id)) {
      const window = await this.initializePopupWindow({
        itemInfo,
        url: `#/headless/import-csv?systemWindowId=0&itemId=${itemInfo.id}&isInvokedUsingToolbar=${isInvokedUsingToolbar}`,
        height: 600,
        width: 1300,
        minWidth: 650,
        modal: false,
      });

      let systemWindowId = WindowManager.getWindowByUid(itemInfo.id).systemWindowId;

      if (isInvokedUsingToolbar)
      {
        window.on('close', async () => {
          let v=new VariableBase(minsky.canvas.item)
          const currentItemId = await v.id();
          const currentItemName = await v.name();
          // We do this check because item focus might have changed when importing csv if user decided to work on something else

          if (currentItemId === itemInfo.id && currentItemName === importCSVvariableName
             ) {
            minsky.canvas.deleteItem();
          }
        });
      }       
      
      window.loadURL(
        WindowManager.getWindowUrl(
          `#/headless/import-csv?systemWindowId=${systemWindowId}&itemId=${itemInfo.id}&isInvokedUsingToolbar=${isInvokedUsingToolbar}&examplesPath=${join(dirname(app.getAppPath()),'examples','data')}`
        )
      );
    }
  }

  static async cut() {
    minsky.cut();
    await CommandsManager.requestRedraw();
  }

  static async copy() {
    minsky.copy();
    await CommandsManager.requestRedraw();
  }

  static async paste() {
    minsky.paste();
    await CommandsManager.requestRedraw();
  }

  static changeWindowBackgroundColor = async (color: string) => {
    const { style, r, g, b } = Functions.getBackgroundStyle(color);
    WindowManager.activeWindows.forEach((window) => {
      window.context.webContents.insertCSS(style);
    });

    minsky.canvas.backgroundColour.$properties({r: r/255, g: g/255, b: b/255, a: 1});
    await CommandsManager.requestRedraw();
  };

  private static async defaultSaveOptions(): Promise<SaveDialogOptions> {
    let defaultExtension=await minsky.model.defaultExtension();
    return {
              filters: [
                {
                  name: defaultExtension,
                  extensions: [defaultExtension.slice(1)],
                },
                { name: 'All', extensions: ['*'] },
              ],
              defaultPath:
                this.currentMinskyModelFilePath ||
                `model${defaultExtension}`,
              properties: ['showOverwriteConfirmation'],
    }
  }

  static async save() {
    if (this.currentMinskyModelFilePath) {
      await minsky.save(this.currentMinskyModelFilePath);
    }
    else
      await this.saveAs();
  }
    
  static async saveAs() {
    const saveDialog = await dialog.showSaveDialog(await CommandsManager.defaultSaveOptions());
    
    const { canceled, filePath: filePath } = saveDialog;
    
    if (canceled || !filePath) {
      return;
    }

    WindowManager.getMainWindow().setTitle(filePath);
    this.currentMinskyModelFilePath=filePath;
    minsky.save(filePath);
    minsky.setAutoSaveFile(filePath+'#');
    ipcMain.emit(events.ADD_RECENT_FILE, null, filePath);
  }
  
  static async editHandleDescription(ravel: Ravel, handleIndex: number) {
    const description = await ravel.handleDescription(handleIndex);
    const window=WindowManager.createPopupWindowWithRouting({
        title: `Handle Description`,
        url: `#/headless/edit-handle-description?command=${ravel.$prefix()}&handleIndex=${handleIndex}&description=${encodeURIComponent(description)}`,
        height: 90,
        width: 300,
      });
      Object.defineProperty(window,'dontCloseOnReturn',{value: true,writable:false});
    }

  static async saveHandleDescription(payload: HandleDescriptionPayload) {
    (new Ravel(payload.command)).setHandleDescription(payload.handleIndex, payload.description);
  }

  static newPubTab(): Promise<void> {
    const window=WindowManager.createPopupWindowWithRouting({
      title: `New Publication Tab`,
      url: `#/headless/new-pub-tab?type=new&command=`,
      height: 90,
      width: 300,
    });
    return new Promise((resolve)=>{
      window.once('closed',()=>resolve(null));
    });
  }
  
  static renamePubTab(command: string): Promise<void> {
    const window=WindowManager.createPopupWindowWithRouting({
      title: `Rename Publication Tab`,
      url: `#/headless/new-pub-tab?type=rename&command=${command}`,
      height: 90,
      width: 300,
    });
    return new Promise((resolve)=>{
      window.once('closed',()=>resolve(null));
    });
  }
  
  static async editHandleDimension(ravel: Ravel, handleIndex: number) {
    const type = await ravel.dimensionType();
    const units = await ravel.dimensionUnitsFormat();

    const window=WindowManager.createPopupWindowWithRouting({
        title: `Handle Dimension`,
        url: `#/headless/edit-handle-dimension?command=${ravel.$prefix()}&handleIndex=${handleIndex}&type=${type}&units=${encodeURIComponent(units)}`,
        height: 180,
        width: 300,
      });
      Object.defineProperty(window,'dontCloseOnReturn',{value: true,writable:false});
  }

  static async saveHandleDimension(payload: HandleDimensionPayload) {
    (new Ravel(payload.command)).setDimension(payload.handleIndex, payload.type, payload.units);
  }

  static async pickSlices(ravel: Ravel, handleIndex: number) {
    const window=WindowManager.createPopupWindowWithRouting({
      title: `Pick slices`,
      url: `#/headless/pick-slices?command=${ravel.$prefix()}&handleIndex=${handleIndex}`,
      height: 400,
      width: 400,
    });
    Object.defineProperty(window,'dontCloseOnReturn',{value: true,writable:false});
  }

  static async lockSpecificHandles(ravel: Ravel) {
    let allLockHandles = await ravel.lockGroup.allLockHandles();
    if(Object.keys(allLockHandles).length === 0) {
      minsky.canvas.lockRavelsInSelection();
      allLockHandles = await ravel.lockGroup.allLockHandles();
      if(Object.keys(allLockHandles).length === 0) return;
    }

    const lockgroup = await ravel.lockGroup.$properties();
    if(lockgroup.handleLockInfo.length === 0) {
      ravel.lockGroup.setLockHandles(allLockHandles);
    }
    const ravelNames = await ravel.lockGroup.ravelNames();
    const window=WindowManager.createPopupWindowWithRouting({
      title: `Lock specific handles`,
      url: `#/headless/lock-handles?handleLockInfo=${encodeURIComponent(JSON5.stringify(lockgroup.handleLockInfo))}&ravelNames=${encodeURIComponent(ravelNames.join())}&lockHandles=${encodeURIComponent(allLockHandles.join())}`,
      height: 200,
      width: 600,
    });
    Object.defineProperty(window,'dontCloseOnReturn',{value: true,writable:false});
  }

  static async applyPreferences() {
    const {
      enableMultipleEquityColumns,
      godleyTableShowValues,
      godleyTableOutputStyle,
      font,
      numBackups,
    } = StoreManager.store.get('preferences');
    minsky.setGodleyDisplayValue(godleyTableShowValues,godleyTableOutputStyle);
    minsky.multipleEquities(enableMultipleEquityColumns);
    minsky.defaultFont(font);
    minsky.numBackups(numBackups);
    RecentFilesManager.updateNumberOfRecentFilesToDisplay();
  }

  // handler for downloading Ravel and installing it
  static downloadRavel(event,item,webContents) {
    switch (process.platform) {
    case 'win32':
      const savePath=dirname(process.execPath)+'/libravel.dll';
      if (existsSync(savePath))
        renameSync(savePath,dirname(process.execPath)+'/deleteme');
      item.setSavePath(savePath);
      break;
    case 'darwin':
    case 'linux':
      mkdirSync(homedir()+'/.ravel',{recursive: true});
      item.setSavePath(homedir()+'/.ravel/libravel.so');
      break;
    default:
      break;
    }

    let progress=new ProgressBar({text:"Downloading Ravel",value: 0, indeterminate:false, closeOnComplete: true,});

    // handler for when download completed
    item.once('done', (event,state)=>{
      progress.close();

      if (state==='completed') {
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: 'Ravel plugin updated successfully - restart Ravel to use',
          type: 'info',
        });
        app.relaunch();
        app.quit();
      } else {
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: `Ravel plugin update failed: ${state}`,
          type: 'error',
        });
        webContents.close();
      }
    });

    // handler for updating progress bar
    item.on('updated',(event,state)=>{
      switch (state) {
      case 'progressing':
        if (!progress?.isCompleted())
          progress.value=100*item.getReceivedBytes()/item.getTotalBytes();
        break;
      case 'interrupted':
        //item.resume();
        break;
      }
    });
  }

  // handler for downloading Minsky
  static downloadMinsky(event,item,webContents) {
    item.setSavePath(join(tmpdir(),item.getFilename()));

    let progress=new ProgressBar({text:"Downloading Ravel application",value: 0, indeterminate:false, closeOnComplete: true,});

    // handler for when download completed
    item.once('done', (event,state)=>{
      progress.close();

      if (state==='completed') {
        switch (process.platform) {
        case 'win32':
          spawn(item.getSavePath(),{detached: true, stdio: 'ignore'});
          app.quit();
          break;
        case 'darwin':
          spawn('open '+item.getSavePath(),{detached: true, shell: true, stdio: 'ignore'});
          app.quit();
        case 'linux': // nothing to be done, updates happen via OBS
          webContents.close();
          break;
        }
      } else {
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: `Ravel update failed: ${state}`,
          type: 'error',
        });
        webContents.close();
      }
    });

    // handler for updating progress bar
    item.on('updated',(event,state)=>{
      switch (state) {
      case 'progressing':
        if (!progress?.isCompleted())
          progress.value=100*item.getReceivedBytes()/item.getTotalBytes();
        break;
      case 'interrupted':
        //item.resume();
        break;
      }
    });
  }

  static startCSVDownload(payload: DownloadCSVPayload) {
    const window=this.createDownloadWindow();

    return new Promise<string>((resolve, reject) => {
      window.webContents.session.on('will-download', (e,i,w) => this.downloadCSV(e,i,w, resolve, reject));
      window.webContents.downloadURL(payload.url);
    });
  }


  static downloadCSV(event,item,webContents, downloadResolve, downloadReject) {
    const extension = extname(item.getFilename());
    let savePath = join(tmpdir(),`downloaded${extension}`);
    item.setSavePath(savePath);

    let progress = new ProgressBar({text:"Downloading CSV File",value: 0, indeterminate:false, closeOnComplete: true});

    // handler for when download completed
    item.once('done', (event,state)=>{
      if (state !== 'completed') {
        const message = `CSV file download failed: ${state}`;

        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: message,
          type: 'error',
        });

        downloadReject(message);
      }
      progress.close();
      webContents.close();

      if(extension === '.zip') {
        decompress(savePath, tmpdir()).then(files => {
          if(files.length > 0) {
            // preferentially find csv or txt file, use first file otherwise
            const csvFile = files.find(f => extname(f.path) === '.csv') || files.find(f => extname(f.path) === '.txt') || files[0];
            const innerExtension = extname(csvFile.path);
            savePath = join(tmpdir(),`downloaded${innerExtension}`);
            renameSync(join(tmpdir(),csvFile.path),savePath);

            downloadResolve(savePath);
          } else {
            downloadReject('Empty zip provided.');
          }
      });
      } else {
        downloadResolve(savePath);
      }
    });

    // handler for updating progress bar
    item.on('updated',(event,state) => {
      switch (state) {
      case 'progressing':
        if (!progress?.isCompleted())
          progress.value=100*item.getReceivedBytes()/item.getTotalBytes();
        break;
      case 'interrupted':
        //item.resume();
        break;
      }
    });
  }

  static createDownloadWindow() {
    return WindowManager.createWindow({
      width: 500,
      height: 700,
      title: 'Download',
      modal: false,
    });
  }
  
  static async upgrade(installCase: InstallCase=InstallCase.theLot) {
    const window=this.createDownloadWindow();

    // handler for when user has logged in to initiate upgrades.
    window.webContents.on('did-navigate',async ()=>{
      const urlString=window.webContents.getURL();
      if (urlString=='https://www.patreon.com/')
        // user has logged out
        window.close()
    });
    window.webContents.on('did-redirect-navigation', async (event)=>{
      const installables=await window.webContents.executeJavaScript('document.getElementById("installables")?.innerText');
      if (installables) {
        let params=new URLSearchParams(installables);
        let minskyFile=params.get('minsky-asset');
        let ravelFile=params.get('ravel-asset');
        if (minskyFile && installCase==InstallCase.theLot) {
          let minskyVersionRE=/(\d+)\.(\d+)\.(\d+)([.-])/;
          let [,major,minor,patch]=minskyVersionRE.exec(minskyFile);
          let [,currMajor,currMinor,currPatch,terminator]=minskyVersionRE.exec((await minsky.minskyVersion())+'.');
          if (+major>+currMajor || major===currMajor &&
              (+minor>+currMinor || minor===currMinor && +patch>+currPatch) ||
              terminator==='-' && // currently a beta release, so install if release nos match (since betas precede releases)
              major===currMajor && minor===currMinor && patch==currPatch
             ) {
            if (ravelFile) { // stash ravel upgrade to be installed on next startup
              StoreManager.store.set('ravelPlugin',ravelFile);
            }
            window.webContents.session.on('will-download',this.downloadMinsky);
            window.webContents.downloadURL(minskyFile);
            return;
          }
        }
        if (ravelFile) {
          // currently on latest, so reinstall ravel
          window.webContents.session.on('will-download',this.downloadRavel);      
          window.webContents.downloadURL(ravelFile);
          return;
        }
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: "Everything's up to date, nothing to do.\n"+
            "If you're trying to download the Ravel plugin, please ensure you are logged into an account subscribed to Ravel Fan or Explorer tiers.",
          type: 'info',
        });
        window.close();
      }
    });

    let clientId='I9sn5lKdemBdh8uTNA7H7YiplxQk3gI-pP0I9_2g1tcbE88T2C3Z9wOvoy51I4-U';
    // need to pass what platform we are
    let state;
    switch (process.platform) {
    case 'win32':
      state={system: 'windows', distro: '', version: '', arch:'', previous: ''};
      break;
    case 'darwin':
      state={system: 'macos', distro: '', version: '', arch: `${process.arch}`, previous: ''};
      break;
    case 'linux':
      state={system: 'linux', distro: '', version: '',arch:'', previous: ''};
      // figure out distro and version from /etc/os-release
      let aexec=promisify(exec);
      let distroInfo=await aexec('grep ^ID= /etc/os-release');
      // value may or may not be quoted
      let extractor=/.*=['"]?([^'"\n]*)['"]?/;
      state.distro=extractor.exec(distroInfo.stdout)[1];
      distroInfo=await aexec('grep ^VERSION_ID= /etc/os-release');
      state.version=extractor.exec(distroInfo.stdout)[1];
      break;
    default:
      dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
            message: `In app update is not available for your operating system yet, please check back later`,
            type: 'error',
      });
      window.close();
      return;
      break;
    }
    if (await minsky.ravelAvailable() && installCase==InstallCase.previousRavel) 
      state.previous=/[^:]*/.exec(await minsky.ravelVersion())[0];
    let encodedState=encodeURI(JSON.stringify(state));
    // load patreon's login page
    window.loadURL(`https://www.patreon.com/oauth2/authorize?response_type=code&client_id=${clientId}&redirect_uri=https://ravelation.hpcoders.com.au/ravel-downloader.cgi&scope=identity%20identity%5Bemail%5D&state=${encodedState}`);
  }
  
}
