import {  importCSVerrorMessage, } from '@minsky/shared';
import { Utility } from '../utility';
import { dialog } from 'electron';
import * as elog from 'electron-log';
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

var errorDialog=(message: string)=>{};
/** enable popup display of exception thrown by backend */
export function setErrorDialog(f: (message: string)=>void) {errorDialog=f}

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
      if (error?.message) errorDialog(error.message);
      return error?.message;
    }
  }
}

export class CppClass
{
  protected prefix: string;
  constructor(prefix: string) {this.prefix=prefix+"/";}
  protected callMethod(method: string,...args)
  {
    return backend(this.prefix+method, ...args);
  }
  public properties(...args) {return backend(this.prefix, ...args);}
  public getPrefix(): string {return this.prefix;}
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

export class Set<Key> extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  insert(key: Key) {this.callMethod("@insert",key);}
  erase(key: Key) {this.callMethod("@erase",key);}
};

