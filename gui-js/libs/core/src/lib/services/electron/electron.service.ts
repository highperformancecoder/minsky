import { Injectable } from '@angular/core';
import { events, MinskyProcessPayload, DescriptionPayload, HandleDescriptionPayload, HandleDimensionPayload, PickSlicesPayload, LockHandlesPayload } from '@minsky/shared';
import { ipcRenderer, remote } from 'electron';
import isElectron from 'is-electron';
import {Minsky, CppClass} from '@minsky/shared';

@Injectable({
  providedIn: 'root',
})
export class ElectronService {
  ipcRenderer: typeof ipcRenderer;
  remote: typeof remote;
  isElectron = isElectron? isElectron(): false;
  minsky: Minsky;
  
  constructor() {
    if (this.isElectron) {
      this.ipcRenderer = (<any>window).require('electron').ipcRenderer;
      this.remote = (<any>window).require('electron').remote;
      this.minsky=new Minsky("/minsky");
      CppClass.backend=async (...args)=>{
        return await this.ipcRenderer.invoke(events.BACKEND, ...args);
      }
    }
  }

  async saveDescription(payload: DescriptionPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_DESCRIPTION, payload);
  }

  async saveHandleDescription(payload: HandleDescriptionPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_HANDLE_DESCRIPTION, payload);
  }

  async saveHandleDimension(payload: HandleDimensionPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_HANDLE_DIMENSION, payload);
  }

  async savePickSlices(payload: PickSlicesPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_PICK_SLICES, payload);
  }

  async saveLockHandles(payload: LockHandlesPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_LOCK_HANDLES, payload);
  }

  async sendMinskyCommandAndRender(
    payload: MinskyProcessPayload,
    customEvent: string = null
  ): Promise<unknown> {
    try {
      if (this.isElectron) {
        if (customEvent) {
          return await this.ipcRenderer.invoke(customEvent, {
            ...payload,
            command: payload.command.trim(),
          });
        }

        return await this.ipcRenderer.invoke(events.MINSKY_PROCESS, {
          ...payload,
          command: payload.command.trim(),
        });
      }
    } catch (error) {
      console.error(
        '🚀 ~ file: electron.service.ts ~ line 43 ~ ElectronService ~ error',
        error,
        payload
      );
    }
  }
}
