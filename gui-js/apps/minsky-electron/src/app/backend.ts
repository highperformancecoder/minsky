import {
  importCSVerrorMessage,
  version,
} from '@minsky/shared';
import { Utility } from './utility';
import { WindowManager } from './managers/WindowManager';
import * as elog from 'electron-log';
import { dialog, ipcMain, shell } from 'electron';
import { join } from 'path';

const JSON5 = require('json5');

const addonDir = Utility.isPackaged()
  ? '../node-addons'
  : '../../node-addons';

var log=elog;
if (!Utility.isDevelopmentMode()) { //clobber logging in production
  log.info=function(...args: any[]){};
};

function logFilter(c: string) {
  const logFilter=["mouseMove$", "requestRedraw$"];
  for (var i in logFilter)
    if (c.match(logFilter[i])) return false;
  return true;
}

let restService = null;
try {
  restService = require('bindings')(join(addonDir, 'minskyRESTService.node'));
} catch (error) {
  log.error(error);
}

restService.setMessageCallback(function (msg: string, buttons: string[]) {
  if (msg)
      return dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
      message: msg,
      type: 'info',
      buttons: buttons,
    });
});

restService.setBusyCursorCallback(function (busy: boolean) {
  WindowManager.getMainWindow().webContents.insertCSS(
    busy ? 'html body {cursor: wait}' : 'html body {cursor: default}'
  );
});

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
    if (args.length>0)
      arg=JSON5.stringify(args);
    console.log(command, arg);
    const response = restService.call(command, arg);
    console.log(response);
    if (logFilter(command))
      log.info('Rest API: ',command,arg,"=>",response);
    return JSON5.parse(response);
  } catch (error) {
    log.error('Rest API: ',command,arg,'=>Exception caught: ' + error?.message);
    if (!dialog) throw error; // rethrow to force error in jest environment
    if (command === '/minsky/canvas/item/importFromCSV') {
      return importCSVerrorMessage;
    } else {
        if (error?.message)
            dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
                message: error.message,
                type: 'error',
            });
        return error?.message;
    }
  }
}

if (backend("/minsky/minskyVersion")!=version)
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


class CppClass
{
  private prefix: string;
  constructor(prefix: string) {this.prefix=prefix+"/";}
  protected callMethod(method: string,...args)
  {
    return backend(this.prefix+method, ...args);
  }
};

export class Canvas extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  
};

export class Minsky extends CppClass
{
  public canvas: Canvas;
  constructor(prefix: string) {
    super(prefix);
    this.canvas=new Canvas(prefix+"/canvas");
  }
  load(file: string) {this.callMethod("load",file);}
  save(file: string) {this.callMethod("save",file);}
};

export const minsky=new Minsky("/minsky");
