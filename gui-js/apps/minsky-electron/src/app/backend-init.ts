import {CppClass, events, Utility, version } from '@minsky/shared';
import { WindowManager } from './managers/WindowManager';
import { dialog, shell } from 'electron';
import * as ProgressBar from 'electron-progressbar';
import * as JSON5 from 'json5';
import * as elog from 'electron-log';

const log=elog? elog: console;
if (!Utility.isDevelopmentMode()) { //clobber logging in production
  log.info=function(...args: any[]){};
}

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
export async function backend(command: string, ...args: any[]): Promise<any> {
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

    let response=await restService.call(command, arg);
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

let progressBar;
let progress={text:"", value:0, indeterminate: false};
let initProgressBar;

if ("JEST_WORKER_ID" in process.env) {
  restService.setMessageCallback(function (msg: string, buttons: string[]) {
    log.info(msg);
    return 0;
  });
  restService.setBusyCursorCallback(function (busy: boolean) {});
} else {
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
    if (!initProgressBar && busy)
      initProgressBar=setTimeout(()=>{
        progressBar=new ProgressBar(progress);
        progressBar.value=progress.value;
        initProgressBar=null;
      }, 3000);
    if (!busy) {
      clearTimeout(initProgressBar);
      initProgressBar=null;
      if (progressBar) {
        progressBar.setCompleted();
        progressBar.close();
        progressBar=null;
        progress.text="";
        progress.value=0;
      }
    }
  });

  restService.setProgressCallback(function (title: string, val: number) {
    progress.text=title;
    progress.value=val;
    if (progressBar)
    {
      progressBar.text=title;
      progressBar.value=val;
    }
  });
}

// Sanity checks before we get started

export function sanityCheck()
{
  setTimeout(async()=>{
  if (await backend("/minsky/minskyVersion")!==version)
    setTimeout(()=>{
      dialog.showMessageBoxSync({
        message: "Mismatch of front end and back end versions",
        type: 'warning',
      });
    },1000);

  if (await backend("/minsky/ravelExpired"))
    setTimeout(()=>{
      const button=dialog.showMessageBoxSync({
        message: "Your Ravel license has expired",
        type: 'warning',
        buttons: ["OK","Upgrade"],
      });
      if (button==1)
        shell.openExternal("https://ravelation.hpcoders.com.au");
  },1000);
  
}, 1);
}

// load icon resources needed for GUI
export function loadResources()
{
  const assetsDir=
        Utility.isDevelopmentMode() && !Utility.isPackaged()
        ? __dirname+'/assets'
        : process.resourcesPath+'/assets';

  backend('/minsky/setGodleyIconResource',assetsDir+'/godley.svg');
  backend('/minsky/setGroupIconResource',assetsDir+'/group.svg');
  backend('/minsky/setRavelIconResource',assetsDir+'/ravel-logo.svg');
  backend('/minsky/setLockIconResource',assetsDir+'/locked.svg',assetsDir+'/unlocked.svg');
}
