/**
 * This module is responsible on handling all the inter process communications
 * between the frontend to the electron backend.
 */

import {
  AppLayoutPayload,
  CanvasItem,
  ChangeTabPayload,
  CppClass,
  events,
  HandleDescriptionPayload,
  HandleDimensionPayload,
  MinskyProcessPayload,
  minsky,
  RenderNativeWindow,
  ImportStockPayload,
  GodleyIcon,
  DownloadCSVPayload,
  VariableBase,
} from '@minsky/shared';
import { BrowserWindow, dialog, ipcMain } from 'electron';
import { BookmarkManager } from '../managers/BookmarkManager';
import { CommandsManager } from '../managers/CommandsManager';
import { ContextMenuManager } from '../managers/ContextMenuManager';
import { GodleyMenuManager } from '../managers/GodleyMenuManager';
import { KeyBindingsManager } from '../managers/KeyBindingsManager';
import { RecentFilesManager } from '../managers/RecentFilesManager';
import { RecordingManager } from '../managers/RecordingManager';
import { StoreManager, MinskyPreferences } from '../managers/StoreManager';
import { WindowManager } from '../managers/WindowManager';
import {restService} from '../backend-init';

//const logUpdateEvent = debug('minsky:electron_update_event');

export default class ElectronEvents {
  static bootstrapElectronEvents(): Electron.IpcMain {
    return ipcMain;
  }
}

ipcMain.handle(events.LOG_MESSAGE, async (event, message: string)=>{
  return await CppClass.logMessage(message);
});

ipcMain.handle(events.BACKEND, async (event, ...args: any[])=>{
  return await CppClass.backend(...args);
});

ipcMain.handle(events.BACKEND_SYNC, async (event, ...args: any[])=>{
  return CppClass.backendSync(...args);
});

ipcMain.handle(events.LOG, (event, msg:string)=>{console.log(msg);});

ipcMain.handle('cancel-progress',()=>{restService.cancelProgress();});

ipcMain.handle(events.GET_CURRENT_WINDOW, (event) => {
  let window=BrowserWindow.fromWebContents(event.sender);
  return {
    id: window.id,
    dontCloseOnEscape: window.hasOwnProperty("dontCloseOnEscape"),
    dontCloseOnReturn: window.hasOwnProperty("dontCloseOnReturn"),
  };
});

ipcMain.handle(events.CLOSE_WINDOW, (event) => {
  BrowserWindow.fromWebContents(event.sender).close();
});

ipcMain.handle(events.OPEN_FILE_DIALOG, async (event, options) => {
  const fileDialog = await WindowManager.showOpenDialog(options);

  if (fileDialog.canceled || !fileDialog.filePaths) return "";
  if (options?.properties?.includes('multiSelections'))
    return fileDialog.filePaths.map((x)=>x.toString());
  return fileDialog.filePaths[0].toString();
});

ipcMain.handle(events.SAVE_FILE_DIALOG, async (event, options) => {
  const fileDialog = await WindowManager.showSaveDialog(options);
  if (fileDialog.canceled) return "";
  return fileDialog.filePath;
});

ipcMain.handle(events.SHOW_MESSAGE_BOX, async (event, options) => {
  return await dialog.showMessageBoxSync(options);
});

ipcMain.on(events.SET_BACKGROUND_COLOR, async (event, { color }) => {
  if (color) {
    StoreManager.store.set('backgroundColor', color);
  }
  await CommandsManager.changeWindowBackgroundColor(
    StoreManager.store.get('backgroundColor')
  );
});

ipcMain.on(events.CREATE_MENU_POPUP, (event, data) => {
  WindowManager.createPopupWindowWithRouting(data);
});

ipcMain.on(
  events.APP_LAYOUT_CHANGED,
  async (event, payload: AppLayoutPayload) => {
    if (event.sender.id === WindowManager.getMainWindow().id) {
      WindowManager.onAppLayoutChanged(payload);
      WindowManager.renderFrame();
    }
  }
);

ipcMain.handle(events.CHANGE_MAIN_TAB, async (event, payload: ChangeTabPayload) => {
  return await WindowManager.setCurrentTab(new RenderNativeWindow(payload.newTab));
});

ipcMain.on(events.UPDATE_BOOKMARK_LIST, async (event) => {
  await BookmarkManager.updateBookmarkList();
});

ipcMain.handle(events.NEW_PUB_TAB, async (event) => {
  return await CommandsManager.newPubTab();
});

ipcMain.on(
  events.INIT_MENU_FOR_GODLEY_VIEW,
  async (
    event,
    parameters: { window: BrowserWindow; itemInfo: CanvasItem }
  ) => {
    const menu = await GodleyMenuManager.createMenusForGodleyView(
      parameters.window,
      parameters.itemInfo
    );
    WindowManager.storeWindowMenu(parameters.window, menu);
  }
);

ipcMain.handle(
  events.GODLEY_VIEW_MOUSEDOWN,async (event, payload: MinskyProcessPayload) => {
    let window=new GodleyIcon(minsky.namedItems.elem(payload.command)).popup;
    GodleyMenuManager.mouseDown(window,payload.mouseX,payload.mouseY);
  }
);

ipcMain.handle(
  events.GODLEY_VIEW_IMPORT_STOCK,async (event, payload: ImportStockPayload) => {
    let window=new GodleyIcon(minsky.namedItems.elem(payload.command)).popup;
    GodleyMenuManager.importStock(window, payload.columnIndex, event);
  }
);

ipcMain.on(
  events.REFRESH_ALL_GODLEY_POPUPS, async (event) => {
    WindowManager.refreshAllGodleyPopups();
  }
);

ipcMain.on(events.ADD_RECENT_FILE, (event, filePath: string) => {
  RecentFilesManager.addFileToRecentFiles(filePath);
});

ipcMain.handle(
  events.KEY_PRESS,
  async (event, payload: MinskyProcessPayload) => {
    // this is a asynchronous handler for events.KEY_PRESS
    return await KeyBindingsManager.handleOnKeyPress(payload);
  }
);

ipcMain.handle(
  events.SAVE_HANDLE_DESCRIPTION,
  async (event, payload: HandleDescriptionPayload) => {
    return await CommandsManager.saveHandleDescription(payload);
  }
);

ipcMain.handle(
  events.SAVE_HANDLE_DIMENSION,
  async (event, payload: HandleDimensionPayload) => {
    return await CommandsManager.saveHandleDimension(payload);
  }
);

ipcMain.handle(
  events.CURRENT_TAB_POSITION,
  async (event)=>{
    return await WindowManager.currentTab?.position();
  }
);

ipcMain.handle(
  events.CURRENT_TAB_MOVE_TO,
  async (event, pos)=>{
    return WindowManager.currentTab.moveTo(pos[0],pos[1]);
  }
);

ipcMain.on(events.KEY_PRESS, async (event, payload: MinskyProcessPayload) => {
  // this is a synchronous handler for events.KEY_PRESS
  event.returnValue = await KeyBindingsManager.handleOnKeyPress(payload);
});

ipcMain.handle(events.GET_PREFERENCES, () => {
  return StoreManager.store.get('preferences');
});

ipcMain.handle(
  events.UPDATE_PREFERENCES,
  async (event, preferences: MinskyPreferences) => {
    StoreManager.store.set('preferences', preferences);
    await CommandsManager.applyPreferences();
    return;
  }
);

ipcMain.handle(events.NEW_SYSTEM, async () => {
  await CommandsManager.createNewSystem();
  return;
});

ipcMain.handle(
  events.IMPORT_CSV,
  async (event) => {
    let v=new VariableBase(minsky.canvas.itemFocus);
    CommandsManager.importCSV(minsky.variableValues.elem(await v.valueId()), true);
    return;
  }
);

ipcMain.handle(
  events.IMPORT_CSV_TO_DB,
  async (event, {dropTable}) => {
    CommandsManager.importCSV(minsky.databaseIngestor, false, dropTable);
    return;
  }
);

ipcMain.on(events.CONTEXT_MENU, async (event, { x, y, type, command}) => {
  await ContextMenuManager.initContextMenu(event, x, y, type, command);
});

ipcMain.on(events.CLICK_MENU, async (event, { x, y, type, command}) => {
  await ContextMenuManager.initClickMenu(event, x, y, type, command);
});

ipcMain.on(
  events.DISPLAY_MOUSE_COORDINATES,
  async (event, { mouseX, mouseY }) => {
    WindowManager.showMouseCoordinateWindow({ mouseX, mouseY });
  }
);

ipcMain.on(
  events.HELP_FOR,
  async (event, { classType }) => {
    CommandsManager.loadHelpFile(classType);
  }
);

ipcMain.on(events.DOUBLE_CLICK, async (event, payload) => {
  await CommandsManager.handleDoubleClick(payload);
});

ipcMain.on(events.LOG_SIMULATION, async (event, selectedItems: string[]) => {
  await CommandsManager.logSimulation(selectedItems);
});

ipcMain.handle(events.RECORD, async (event) => {
  await RecordingManager.handleRecord();
});
ipcMain.handle(events.RECORDING_REPLAY, async (event) => {
  await RecordingManager.handleRecordingReplay();
});
ipcMain.handle(events.DISPLAY_INIT_HELP, (event)=> {
  CommandsManager.loadHelpFile('tensor-init');
});

ipcMain.handle(
  events.DOWNLOAD_CSV,
  async (event, payload: DownloadCSVPayload) => {
    const savePath = await CommandsManager.startCSVDownload(payload);
    return savePath;
  }
);

ipcMain.handle(events.OPEN_URL, (event,options)=> {
  let window=WindowManager.createWindow(options);
  window.loadURL(options.url);
});
