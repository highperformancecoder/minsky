import {
  ActiveWindow,
  AppLayoutPayload,
  CreateWindowPayload,
  Functions,
  minsky,
  OPEN_DEV_TOOLS_IN_DEV_BUILD,
  rendererAppName,
  rendererAppURL,
  RenderNativeWindow,
  Utility,
} from '@minsky/shared';
import { StoreManager } from './StoreManager';
import { BrowserWindow, dialog, Menu, screen } from 'electron';
import log from 'electron-log';
import os from 'os';
import { join } from 'path';
import { format } from 'url';

//const logWindows = debug('minsky:electron_windows');

export class WindowManager {
  static topOffset: number;
  static electronTopOffset: number;
  static leftOffset: number;
  static canvasHeight: number;
  static canvasWidth: number;
  static scaleFactor: number;
  static currentTab=minsky.canvas as RenderNativeWindow;
  
  static activeWindows = new Map<number, ActiveWindow>();
  private static uidToWindowMap = new Map<string, ActiveWindow>();

  static getWindowByUid(uid: string): ActiveWindow {
    return this.uidToWindowMap.get(uid);
  }

  static storeWindowMenu(win: BrowserWindow, menu: Menu) {
    const details = this.activeWindows.get(win.id);
    if (details) {
      details.menu = menu;
    }
    if (Functions.isMacOS()) {
      win.on('focus', function () {
        Menu.setApplicationMenu(menu);
      });
    }
  }

  static setApplicationMenu(win: BrowserWindow) {
    if (Functions.isMacOS()) {
      const details = this.activeWindows.get(win.id);
      if (details) {
        Menu.setApplicationMenu(details.menu);
      }
    }
  }

  static async renderFrame() {
    try
    {
      return this.currentTab?.renderFrame(
                                       {
                                         parentWindowId: this.activeWindows.get(1).systemWindowId.toString(),
                                         offsetLeft: this.leftOffset,
                                         offsetTop: this.topOffset+this.electronTopOffset,
                                         childWidth: this.canvasWidth,
                                         childHeight: this.canvasHeight,
                                         scalingFactor: this.scaleFactor
                                       });
    }
    catch (err) {
      // absorb exceptions, which will mostly be due to bad windows
      console.log(err);
    }
  }
    
  static async setCurrentTab(tab/*: RenderNativeWindow*/) {
    if (this.currentTab!==tab) {
      await this.currentTab?.destroyFrame();
      this.currentTab=tab;
      return this.renderFrame();
    }
  }
  
  static getSystemWindowId(menuWindow: BrowserWindow) {
    const nativeBuffer = menuWindow.getNativeWindowHandle();
    switch (nativeBuffer.length) {
      case 4:
        return BigInt(
          os.endianness() == 'LE'
            ? nativeBuffer.readUInt32LE(0)
            : nativeBuffer.readUInt32BE(0)
        );
      case 8:
        return os.endianness() == 'LE'
          ? nativeBuffer.readBigUInt64LE(0)
          : nativeBuffer.readBigUInt64BE(0);
      default:
        log.error('Unsupported native window handle type');
        return BigInt(0);
    }
  }

  static getMainWindow(): BrowserWindow {
    return this.activeWindows.get(1)?.context; // TODO:: Is this accurate?
  }

  static focusIfWindowIsPresent(uid: string) {
    const windowDetails = this.uidToWindowMap.get(uid);
    if (windowDetails) {
      windowDetails.context.focus();
      return true;
    }
    return false;
  }

  static getWindowUrl(url: string) {
    if (!Utility.isPackaged()) {
      const initialURL = url ? rendererAppURL + url : rendererAppURL;
      return initialURL;
    }

    const path = format({
      pathname: join(__dirname, '..', rendererAppName, 'index.html'),
      protocol: 'file:',
      slashes: true,
    });

    const initialURL = path + (url || '#/');
    return initialURL;
  }

  static createPopupWindowWithRouting(
    payload: CreateWindowPayload,
    // eslint-disable-next-line @typescript-eslint/ban-types
    onCloseCallback?: (ev : Electron.Event) => void
  ): BrowserWindow {
    const window = WindowManager.createWindow(payload, onCloseCallback);
      // strip off leading #, as URL doesn't know how to handle it. Add dummy protocol and host
      const url = new URL(payload.url[0]=='#'? payload.url.slice(1): payload.url,"http://localhost");
      
      url.searchParams.set('systemWindowId',WindowManager.getSystemWindowId(window).toString());
      const relativeUrlString=(payload.url[0]=='#'?'#':'') +  url.pathname+'?'+url.searchParams.toString();
      window.loadURL(this.getWindowUrl(relativeUrlString)); 
    return window;
  }

  static closeWindowByUid(uid: string) {
    const windowDetails = this.uidToWindowMap.get(uid);
    if (windowDetails) {
      this.uidToWindowMap.delete(uid);
      windowDetails.context.close();
    }
  }

  static createMenuPopUpAndLoadFile(
    payload: CreateWindowPayload
  ): BrowserWindow {
    const window = WindowManager.createWindow(payload);

    let [path,query]=payload.url.split('?');
    const filePath = format({
      pathname: path,
      search: query,
      protocol: 'file:',
      slashes: true,
    });

    console.log(payload.url);
    console.log(filePath);
    window.loadURL(filePath);
    return window;
  }

  static createWindow(
    payload: CreateWindowPayload,
    onCloseCallback?: (ev : Electron.Event) => void
  ) {
    const { width, height, minWidth, minHeight, title, modal = true, backgroundColor=StoreManager.store.get('backgroundColor'), alwaysOnTop } = payload;

    const childWindow = new BrowserWindow({
      width,
      height,
      minWidth: minWidth || Math.min(width, 300),
      minHeight: minHeight || Math.min(height, 200),
      title,
      resizable: true,
      useContentSize: true,
      minimizable: false,
      show: false,
      parent: modal ? this.getMainWindow() : null,
      modal,
      backgroundColor,
      alwaysOnTop,
      webPreferences: {
        contextIsolation: true,
        preload: join(__dirname, 'preload.js'),
        nodeIntegration: true,
      },
      icon: __dirname + '/assets/favicon.png',
    });

    childWindow.setMenu(null);

    childWindow.once('ready-to-show', () => {
      childWindow.show();
    });

    childWindow.once('page-title-updated', (event) => {
      event.preventDefault();
    });

    /* Dev tools results in lag in handling multiple key inputs. Hence enable only temporarily when needed */
    if (Utility.isDevelopmentMode() && OPEN_DEV_TOOLS_IN_DEV_BUILD) {
      childWindow.webContents.openDevTools({ mode: 'detach', activate: false });
      // command to inspect popup
    }

    const windowId = WindowManager.getSystemWindowId(childWindow);
    const childWindowDetails: ActiveWindow = {
      id: childWindow.id,
      size: childWindow.getSize(),
      isMainWindow: false,
      context: childWindow,
      systemWindowId: windowId,
      menu: null,
    };

    if (payload.uid) {
      this.uidToWindowMap.set(payload.uid, childWindowDetails);
    }

    this.activeWindows.set(childWindow.id, childWindowDetails);
//    logWindows(WindowManager.activeWindows);

    childWindow.on('close', (ev : Electron.Event) => {
      try {
        if (payload?.uid) {
          this.uidToWindowMap.delete(payload.uid);
        }
        if (childWindow?.id) {
          this.activeWindows.delete(childWindow.id);
        }
        if (onCloseCallback) {
          onCloseCallback(ev);
        }
      } catch (error) {
        log.error(error);
      }
    });
    // in the event the webcontents is closed without the containing window being so.
    childWindow.webContents.on('destroyed', ()=> this.activeWindows.delete(childWindow.id));
    return childWindow;
  }

  public static scrollToCenter() {
    // TODO:: Replace this with something cleaner
    this.getMainWindow().webContents.executeJavaScript(
      `var container=document.getElementsByClassName('minsky-canvas-container')[0]; var canvas = container.getElementsByTagName('canvas')[0]; container.scrollTop=canvas.clientHeight/2; container.scrollLeft=canvas.clientWidth/2;`,
      false
    );
  }

  static onAppLayoutChanged(payload: AppLayoutPayload) {

    this.topOffset = Math.round(payload.offset.top);
    this.leftOffset = Math.round(payload.offset.left);
    this.scaleFactor = screen.getPrimaryDisplay().scaleFactor;
    let size=this.getMainWindow().getSize();
    let contentSize=this.getMainWindow().getContentSize();
    this.electronTopOffset = size[1]-contentSize[1];

    this.canvasHeight = payload.drawableArea.height;
    this.canvasWidth = payload.drawableArea.width;

  }

  
  
  static showMouseCoordinateWindow({ mouseX, mouseY }) {
    dialog.showMessageBox(WindowManager.getMainWindow(), {
      message: `MouseX: ${mouseX}, MouseY: ${mouseY}`,
      title: 'Mouse Coordinates',
      type: 'info',
    });
  }
}
