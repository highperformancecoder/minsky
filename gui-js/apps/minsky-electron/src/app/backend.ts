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

var log=elog? elog: console;
if (!Utility.isDevelopmentMode()) { //clobber logging in production
  log.info=function(...args: any[]){};
};

/** returns true if RESTService call is logged in development mode */
function logFilter(c: string) {
  const logFilter=["mouseMove$", "requestRedraw$"];
  for (var i in logFilter)
    if (c.match(logFilter[i])) return false;
  return true;
}

/** REST Service addon */
export var restService = null;
try {
  restService = require('bindings')(join(addonDir, 'minskyRESTService.node'));
} catch (error) {
  log.error(error);
}

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
  WindowManager.getMainWindow()?.webContents?.insertCSS(
    busy ? 'html body {cursor: wait}' : 'html body {cursor: default}'
  );
});

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
    if (args.length>0)
      arg=JSON5.stringify(args);
    const response = restService.call(command, arg);
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
  protected prefix: string;
  constructor(prefix: string) {this.prefix=prefix+"/";}
  protected callMethod(method: string,...args)
  {
    return backend(this.prefix+method, ...args);
  }
  public properties(...args) {return backend(this.prefix, ...args);}
};

export class Item extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  classType(): string {return this.callMethod("classType");}
  static make(prefix: string) {return new Item(prefix);}
}

export class Canvas extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  addOperation(type: string) {return this.callMethod("addOperation",type);}
  getItemAt(x: number, y: number) {return this.callMethod("getItemAt",x,y);}
};

export class Pair<Key, Value extends CppClass> {
  first: Key;
  second: Value;
  constructor(key: Key, value: Value) {
    this.first=key;
    this.second=value;
  }
};

export class Map<Key, Value extends CppClass> extends CppClass
{
  valueType: any;  // stash a reference to the actual type here, for use in a new expression
  constructor(prefix: string, valueType: any) {super(prefix); this.valueType=valueType;}
  elem(key: Key) {
    return new Pair<Key,Value>
      (key,new this.valueType(this.prefix+"@elem/"+JSON5.stringify(key)+"/second"));
  }
  insert(keyValue: Pair<Key, Value>) {this.callMethod("@insert",keyValue);}
  erase(key: Key) {this.callMethod("@erase",key);}
};

export class Minsky extends CppClass
{
  canvas: Canvas;
  namedItems: Map<string, Item>;
  
  constructor(prefix: string) {
    super(prefix);
    this.canvas=new Canvas(prefix+"/canvas");
    this.namedItems=new Map<string, Item>(prefix+"/namedItems",Item);
  }

  nameCurrentItem(name: string) {this.callMethod("nameCurrentItem",name);}
  load(file: string) {this.callMethod("load",file);}
  save(file: string) {this.callMethod("save",file);}
};

/** global backend Minsky object */
export const minsky=new Minsky("/minsky");
