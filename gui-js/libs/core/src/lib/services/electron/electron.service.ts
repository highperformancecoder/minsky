import { Injectable } from '@angular/core';
import { events, MinskyProcessPayload, /*DescriptionPayload,*/ HandleDescriptionPayload, HandleDimensionPayload, PickSlicesPayload,} from '@minsky/shared';
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

  async saveHandleDescription(payload: HandleDescriptionPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_HANDLE_DESCRIPTION, payload);
  }

  async saveHandleDimension(payload: HandleDimensionPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_HANDLE_DIMENSION, payload);
  }

  async savePickSlices(payload: PickSlicesPayload) {
    return await this.ipcRenderer.invoke(events.SAVE_PICK_SLICES, payload);
  }

  async currentTabPosition(): Promise<number[]> {
    return await this.ipcRenderer.invoke(events.CURRENT_TAB_POSITION);
  }

  async currentTabMoveTo(x: number, y: number): Promise<void> {
    return await this.ipcRenderer.invoke(events.CURRENT_TAB_MOVE_TO,[x,y]);
  }

  async record(): Promise<void> {
    return await this.ipcRenderer.invoke(events.RECORD);
  }
  async recordingReplay(): Promise<void> {
    return await this.ipcRenderer.invoke(events.RECORDING_REPLAY);
  }
  
}
