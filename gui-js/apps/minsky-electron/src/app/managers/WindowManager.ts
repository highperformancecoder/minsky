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
import { BrowserWindow, dialog, Menu, OpenDialogOptions, OpenDialogReturnValue, SaveDialogOptions,
         SaveDialogReturnValue, screen } from 'electron';
import log from 'electron-log';
import os from 'os';
import { join, dirname } from 'path';
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
    return WindowManager.uidToWindowMap.get(uid);
  }

  static storeWindowMenu(win: BrowserWindow, menu: Menu) {
    const details = WindowManager.activeWindows.get(win.id);
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
      const details = WindowManager.activeWindows.get(win.id);
      if (details) {
        Menu.setApplicationMenu(details.menu);
      }
    }
  }

  static async renderFrame() {
    try
    {
      return WindowManager.currentTab?.renderFrame(
                                       {
                                         parentWindowId: WindowManager.activeWindows.get(1).systemWindowId.toString(),
                                         offsetLeft: WindowManager.leftOffset,
                                         offsetTop: WindowManager.topOffset+WindowManager.electronTopOffset,
                                         childWidth: WindowManager.canvasWidth,
                                         childHeight: WindowManager.canvasHeight,
                                         scalingFactor: WindowManager.scaleFactor
                                       });
    }
    catch (err) {
      // absorb exceptions, which will mostly be due to bad windows
      console.log(err);
    }
  }
    
  static async setCurrentTab(tab/*: RenderNativeWindow*/) {
    if (WindowManager.currentTab!==tab) {
      await WindowManager.currentTab?.destroyFrame();
      WindowManager.currentTab=tab;
      return WindowManager.renderFrame();
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
    return WindowManager.activeWindows.get(1)?.context; // TODO:: Is WindowManager accurate?
  }

  static focusIfWindowIsPresent(uid: string) {
    const windowDetails = WindowManager.uidToWindowMap.get(uid);
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

  /// If options contains defaultPath that has starts with ':model/'
  /// or ':data/', then the last directory visited with that type is
  /// substituted.
  /// returns the directory key for the StoreManager.
  static processDefaultDirectory(options: OpenDialogOptions|SaveDialogOptions) {
    let splitDefaultPath=/([^\/]*)\/?(.*)/.exec(options.defaultPath);
    let defaultType=splitDefaultPath[1];
    let defaultDirectoryKey="";
    
    switch (defaultType) {
    case ':models':
      defaultDirectoryKey='defaultModelDirectory';
      break;
    case ':data':
      defaultDirectoryKey='defaultDataDirectory';
      break;
    }

    if (defaultDirectoryKey)  {
      let defaultDirectory=StoreManager.store.get(defaultDirectoryKey) as string;
      if (defaultDirectory)
        options['defaultPath']=defaultDirectory+'/'+splitDefaultPath[2];
    }
    return defaultDirectoryKey;
  }
  
  /// wrappers around the standard electron dialogs that saves the directory opened as a defaultPath
  /// if options.defaultPath is set to either models or data.
  static async showOpenDialog(...args: any[])
  {
    let options=args[args.length-1] as OpenDialogOptions;
    let defaultDirectoryKey=this.processDefaultDirectory(options);

    let res: Electron.OpenDialogReturnValue;
    if (args.length>1)
      res=await dialog.showOpenDialog(args[0],options);
    else
      res=await dialog.showOpenDialog(options);
    if (!res.canceled && defaultDirectoryKey) {
      StoreManager.store.set(defaultDirectoryKey,dirname(res.filePaths[0]));
    }
    return res;
  }
  
  /// wrappers around the standard electron dialogs that saves the directory opened as a defaultPath
  /// if options.defaultPath is set to either models or data.
  static async showSaveDialog(...args: any[])
  {
    let options=args[args.length-1] as SaveDialogOptions;
    let defaultDirectoryKey=this.processDefaultDirectory(options);

    let res: Electron.SaveDialogReturnValue;
    if (args.length>1) 
      res=await dialog.showSaveDialog(args[0], options);
    else
      res=await dialog.showSaveDialog(options);
    if (!res.canceled && defaultDirectoryKey) {
      StoreManager.store.set(defaultDirectoryKey,dirname(res.filePath));
    }
    return res;
  }
  
  /// if window already exists attached to \a url, then raise it
  /// @return window if it exists, null otherwise
  static raiseWindow(url: string): BrowserWindow {
    let window=null;
    for (let i of WindowManager.activeWindows) 
      if (i[1].url==url) {
        window=i[1].context;
        break;
      }
    if (window) window.show();
    return window;
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
      window.loadURL(WindowManager.getWindowUrl(relativeUrlString)); 
    return window;
  }

  static closeWindowByUid(uid: string) {
    const windowDetails = WindowManager.uidToWindowMap.get(uid);
    if (windowDetails) {
      WindowManager.uidToWindowMap.delete(uid);
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

    window.loadURL(filePath);
    return window;
  }

  static createWindow(
    payload: CreateWindowPayload,
    onCloseCallback?: (ev : Electron.Event) => void
  ) {
    const { width, height, minWidth, minHeight, title, modal = true, backgroundColor=StoreManager.store.get('backgroundColor'), alwaysOnTop, url } = payload;

    // do not duplicate window if requested and window already exists
    if (payload.raiseIfPresent) {
      const childWindow=WindowManager.raiseWindow(url);
      if (childWindow) return childWindow;
    }
    
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
      parent: modal ? WindowManager.getMainWindow() : null,
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
      url,
    };

    if (payload.uid) {
      WindowManager.uidToWindowMap.set(payload.uid, childWindowDetails);
    }

    WindowManager.activeWindows.set(childWindow.id, childWindowDetails);

    childWindow.on('close', (ev : Electron.Event) => {
      try {
        if (payload?.uid) {
          WindowManager.uidToWindowMap.delete(payload.uid);
        }
        if (childWindow?.id) {
          WindowManager.activeWindows.delete(childWindow.id);
        }
        if (onCloseCallback) {
          onCloseCallback(ev);
        }
      } catch (error) {
        log.error(error);
      }
    });
    // in the event the webcontents is closed without the containing window being so.
    childWindow.webContents.on('destroyed', ()=> WindowManager.activeWindows.delete(childWindow.id));
    return childWindow;
  }

  public static scrollToCenter() {
    // TODO:: Replace WindowManager with something cleaner
    WindowManager.getMainWindow().webContents.executeJavaScript(
      `var container=document.getElementsByClassName('minsky-canvas-container')[0]; var canvas = container.getElementsByTagName('canvas')[0]; container.scrollTop=canvas.clientHeight/2; container.scrollLeft=canvas.clientWidth/2;`,
      false
    );
  }

  static onAppLayoutChanged(payload: AppLayoutPayload) {

    WindowManager.topOffset = Math.round(payload.offset.top);
    WindowManager.leftOffset = Math.round(payload.offset.left);
    WindowManager.scaleFactor = screen.getPrimaryDisplay().scaleFactor;
    if (Functions.isWindows())
      // calculate WindowManager offset internally in C++
      WindowManager.electronTopOffset = 0;
    else
    {
      let size=WindowManager.getMainWindow().getSize();
      let contentSize=WindowManager.getMainWindow().getContentSize();
      WindowManager.electronTopOffset = size[1]-contentSize[1];
    }

    WindowManager.canvasHeight = payload.drawableArea.height;
    WindowManager.canvasWidth = payload.drawableArea.width;

  }

  
  
  static showMouseCoordinateWindow({ mouseX, mouseY }) {
    dialog.showMessageBox(WindowManager.getMainWindow(), {
      message: `MouseX: ${mouseX}, MouseY: ${mouseY}`,
      title: 'Mouse Coordinates',
      type: 'info',
    });
  }
}
