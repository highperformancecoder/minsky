import {
  ActiveWindow,
  minsky,
  OPEN_DEV_TOOLS_IN_DEV_BUILD,
  Functions,
  rendererAppName,
  rendererAppURL,
  version,
  Utility,
} from '@minsky/shared';
import { BrowserWindow, dialog, screen } from 'electron';
import * as log from 'electron-log';
import { isAbsolute, join } from 'path';
import { format } from 'url';
import { ApplicationMenuManager } from './managers/ApplicationMenuManager';
import { BookmarkManager } from './managers/BookmarkManager';
import { CommandsManager } from './managers/CommandsManager';
import { HelpFilesManager } from './managers/HelpFilesManager';
import { RecentFilesManager } from './managers/RecentFilesManager';
import { StoreManager } from './managers/StoreManager';
import { WindowManager } from './managers/WindowManager';
import { backend, backendSync, initialWorkingDirectory, loadResources, sanityCheck } from './backend-init';

export default class App {
  // Keep a global reference of the window object, if you don't, the window will
  // be closed automatically when the JavaScript object is garbage collected.
  static mainWindow: Electron.BrowserWindow;
  static application: Electron.App;
  static BrowserWindow;
  static directlyClose = false;
  static cliArguments=[]; // first argument passed on command line
  
  private static onWindowAllClosed() {
      App.application.quit();
  }
  
  private static async onReady() {
    // This method will be called when Electron has finished
    // initialization and is ready to create browser windows.
    // Some APIs can only be used after this event occurs.

    const helpFilesFolder = Utility.isPackaged()
      ? join(process.resourcesPath, 'minsky-docs')
      : __dirname + '/../../../minsky-docs/';

    backend('minsky.pushFlags');
    await HelpFilesManager.initialize(helpFilesFolder);
    App.initMainWindow();
    await App.initMenu();
    App.loadMainWindow();
    backend('minsky.popFlags');
    if (App.cliArguments.length>1) {
      if (!isAbsolute(App.cliArguments[1]))
        App.cliArguments[1]=join(initialWorkingDirectory,App.cliArguments[1]);
	try
	{
          await CommandsManager.openNamedFile(App.cliArguments[1]);
          BookmarkManager.updateBookmarkList();
	}
	catch (err) {
	// Macs sometimes put extra guff on the command line, so ignore errors
	}
    }
  }

  private static async initMenu() {
    const menu = ApplicationMenuManager.createMainApplicationMenu();
    WindowManager.storeWindowMenu(App.mainWindow, menu);
    RecentFilesManager.initRecentFiles();

    StoreManager.store.onDidChange('recentFiles', () => {
      RecentFilesManager.initRecentFiles();
    });

    await ApplicationMenuManager.buildMenuForInsertOperations();
  }

  private static onActivate() {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (App.mainWindow === null) {
      App.onReady();
    }
  }

  private static initMainWindow() {
    const display = screen.getPrimaryDisplay();
    const workAreaSize = display.workAreaSize;
    const width = Math.round(Math.max(1000, workAreaSize.width * 0.9));
    const height = Math.round(Math.max(600, workAreaSize.height * 0.9));

    // Create the browser window.
    let ravelVersion=minsky.$callMethodSync("ravelVersion");
    App.mainWindow = new BrowserWindow({
      width: width,
      height: height,
      show: false,
      webPreferences: {
        contextIsolation: true,
        preload: join(__dirname, 'preload.js'),
        nodeIntegration: true,
        backgroundThrottling: false,
      },
      x: 0,
      y: 0,
      title: ravelVersion==="unavailable"? 'Minsky':'Ravel',
      icon: __dirname + '/assets/favicon.png',
      resizable: true,
      // autoHideMenuBar: true,
      // useContentSize: true,
      // frame: false,
    });

    App.mainWindow.center();

    if (Utility.isDevelopmentMode() && OPEN_DEV_TOOLS_IN_DEV_BUILD) {
      App.mainWindow.webContents.openDevTools({
        mode: 'detach',
        activate: false,
      });
    }

    // if main window is ready to show, close the splash window and show the main window
    App.mainWindow.once('ready-to-show', () => {
      App.mainWindow.show();
    });

    // Emitted when the window is closed.

    const mainWindowDetails: ActiveWindow = {
      id: App.mainWindow.id,
      size: App.mainWindow.getSize(),
      isMainWindow: true,
      context: App.mainWindow,
      systemWindowId: WindowManager.getSystemWindowId(this.mainWindow),
      menu: null,
    };

    WindowManager.activeWindows.set(App.mainWindow.id, mainWindowDetails);

    CommandsManager.applyPreferences();

    App.mainWindow.on('close', async (e) => {
      if (!App.directlyClose) {
          e.preventDefault();
        const canProceed = await CommandsManager.canCurrentSystemBeClosed();
        if (!canProceed) {
          return;
        }
        
        WindowManager.activeWindows.forEach((window) => {
          if (!window.isMainWindow) {
            window.context.close();
          }
        });
   
          App.directlyClose = true;
          App.mainWindow.close();
      }
      WindowManager.activeWindows.delete(App.mainWindow.id); // Is this needed?
    });

    // TODO: test it on Russell's machine
    // TODO: do the same thing for child windows (godley,plot)
    App.mainWindow.on('focus', async () => {
      await CommandsManager.requestRedraw();
    });

    // App.mainWindow.on('blur', async () => {
    //   await CommandsManager.requestRedraw();
    // });

    App.mainWindow.on('closed', () => {
      // Dereference the window object, usually you would store windows
      // in an array if your app supports multi windows, this is the time
      // when you should delete the corresponding element.
      App.mainWindow = null;
    });
  }

  private static loadMainWindow() {
    // load the index.html of the app.
    if (!App.application.isPackaged) {
      App.mainWindow.loadURL(rendererAppURL);
    } else {
      App.mainWindow.loadURL(
        format({
          pathname: join(__dirname, '..', rendererAppName, 'index.html'),
          protocol: 'file:',
          slashes: true,
        })
      );
    }
  }

  static main(app: Electron.App, browserWindow: typeof BrowserWindow) {

    // when run from npm start, argv[0] is 'electron'
    if (process.argv[0].slice(-8)!=='electron')
      // process CLI options prior to running up any GUI
      for (var arg in process.argv) {
        switch(process.argv[arg]) {
        case '--version': {
            let minskyVersion=backendSync("minsky.minskyVersion");
            if (minskyVersion===version)
            {
              process.stdout.write(`${version}\n`);
              process.exit(0);
            }
            else
            {
              process.stdout.write(`${version}/${minskyVersion}\n`);
              process.exit(1);
            }
          }
          break;
        default:
          // pass argument on for an initial model load
          if (process.argv[arg][0]!=='-')
            App.cliArguments.push(process.argv[arg]);
          break;
        }
      }

    // we pass the Electron.App object and the
    // Electron.BrowserWindow into this function
    // so this class has no dependencies. This
    // makes the code easier to write tests for
    App.BrowserWindow = browserWindow;
    App.application = app;

    App.application.commandLine.appendSwitch('disable-gpu');
    // Rendering was not working on some window's machines without disabling gpu
    App.application.commandLine.appendSwitch('high-dpi-support', '1');
    // This probably supports high-res fonts, but we don't know exactly what implications it has!

    //This effects how display scaling is handled -  if set to 1, then it will ignore the scale factor (always set it to 1).
    // Typically, effects are visible on display resolutions > 2MP. Electron seems to scale down its window
    // when native display resolution is > 2MP by default. If we force to 1, it will not scale down
    let displayScale=backendSync('minsky.canvas.scaleFactor');
    App.application.commandLine.appendSwitch('force-device-scale-factor', displayScale.toString());
    // invert the effect of display scaling on canvas fonts.
    backendSync('minsky.fontScale', (1/displayScale));
    loadResources();
    sanityCheck();
    
    App.application.on('window-all-closed', App.onWindowAllClosed); // Quit when all windows are closed.
    App.application.on('ready', App.onReady); // App is ready to load data
    App.application.on('activate', App.onActivate); // App is activated

    process.on('uncaughtException', (err) => {
      log.error(
        Functions.red(
          `🚀 ~ file: app.ts ~ line 265 ~ App ~ process.on('uncaughtException') ~ err: ${err}`
        )
      );
      if (err?.message) dialog.showErrorBox(err.message, '');
    });
  }
}
