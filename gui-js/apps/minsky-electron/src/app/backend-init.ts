
// See gui-js/libs/shared/src/test-setup.ts for the equivalent definitions for the jest environment.

import {CppClass, events, Utility, version } from '@minsky/shared';
import { WindowManager } from './managers/WindowManager';
import { BookmarkManager } from './managers/BookmarkManager';
import { dialog, shell } from 'electron';
import * as JSON5 from 'json5';
import * as elog from 'electron-log';
import * as path from 'path';

const log=elog? elog: console;
if (!Utility.isDevelopmentMode()) { //clobber logging in production
  log.info=function(...args: any[]){};
}

export const initialWorkingDirectory=process.cwd();
process.chdir(path.dirname(process.execPath));
const addonDir = Utility.isPackaged()
      ? '../../node-addons'
      : '../../node-addons';
/** REST Service addon */
export var restService = null;
try {
  restService = require('bindings')(addonDir+'/minskyRESTService.node');
} catch (error) {
  log.error(error);
}

/** returns true if RESTService call is logged in development mode */
function logFilter(c: string) {
  const logFilter=["mouseMove$", "requestRedraw$"];
  for (var i in logFilter)
    if (c.match(logFilter[i])) return false;
  return true;
}

/** core function to call into C++ object heirarachy */
export function backend(command: string, ...args: any[]) {
  if (!command) {
    log.error('backend called without any command');
    return {};
  }
  if (!restService) {
    log.error('Rest Service not ready');
    return {};
  }
  try {
    var arg='';
    if (args.length>1) {
      arg=JSON5.stringify(args, {quote: '"'});
    } else if (args.length===1) {
      arg=JSON5.stringify(args[0], {quote: '"'});
    }
    CppClass.record(`${command} ${arg}`);

    const response = restService.call(command, arg);
    if (logFilter(command))
      log.info('Rest API: ',command,arg,"=>",response);
    return JSON5.parse(response);
  } catch (error) {
    log.error('Rest API: ',command,arg,'=>Exception caught: ' + error?.message);
    if (!dialog) throw error; // rethrow to force error in jest environment
      if (error?.message && command !== '/minsky/canvas/item/importFromCSV')
        dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
          message: error.message,
          type: 'error',
        });
      return error?.message;
  }
}

CppClass.backend=backend;

restService.setMessageCallback(function (msg: string, buttons: string[]) {
  if (msg && dialog)
    return dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
      message: msg,
      type: 'info',
      buttons: buttons,
    });
  return 0;
});

restService.setBusyCursorCallback(function (busy: boolean) {
  WindowManager.getMainWindow()?.webContents?.send(events.CURSOR_BUSY, busy);
});

var bookmarkRefreshTimer;
restService.setBookmarkRefreshCallback(()=>{
  if (bookmarkRefreshTimer)
    bookmarkRefreshTimer.refresh(); // coalesce repeated calls to refreshBookmarkList
  else
    bookmarkRefreshTimer=setTimeout(()=>{
      bookmarkRefreshTimer=null;
      BookmarkManager.updateBookmarkList();
    },10);
});

// Sanity checks before we get started
if (backend("/minsky/minskyVersion")!==version)
  setTimeout(()=>{
    dialog.showMessageBoxSync({
      message: "Mismatch of front end and back end versions",
      type: 'warning',
    });
  },1000);

if (backend("/minsky/ravelExpired"))
  setTimeout(()=>{
    const button=dialog.showMessageBoxSync({
      message: "Your Ravel license has expired",
      type: 'warning',
      buttons: ["OK","Upgrade"],
    });
    if (button==1)
      shell.openExternal("https://ravelation.hpcoders.com.au");
  },1000);


// load icon resources needed for GUI
export function loadResources()
{
  const assetsDir=
        Utility.isDevelopmentMode() && !Utility.isPackaged()
        ? __dirname+'/assets'
        : process.resourcesPath+'/assets';

  backend('/minsky/pushFlags');
  backend('/minsky/setGodleyIconResource',assetsDir+'/godley.svg');
  backend('/minsky/setGroupIconResource',assetsDir+'/group.svg');
  backend('/minsky/setRavelIconResource',assetsDir+'/ravel-logo.svg');
  backend('/minsky/setLockIconResource',assetsDir+'/locked.svg',assetsDir+'/unlocked.svg');
  backend('/minsky/popFlags');
}
