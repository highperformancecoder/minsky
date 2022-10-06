import {CppClass} from './backend';
import {HandleLockInfo} from '@minsky/shared';

export class RavelLockGroup extends CppClass
{
  constructor(prefix: string) {super(prefix);}
  allLockHandles(): string[] {return this.callMethod("allLockHandles");}
  handleLockInfo(...args: HandleLockInfo[][]): HandleLockInfo {return this.callMethod("handleLockInfo",...args);}
  ravelNames(): string[] {return this.callMethod("ravelNames");}
  setLockHandles(handles: string[]): void {this.callMethod("setLockHandles");}
  validateLockHandleInfo(): void {this.callMethod("validateLockHandleInfo");}
}
