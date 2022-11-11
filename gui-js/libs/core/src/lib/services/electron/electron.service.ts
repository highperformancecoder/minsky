import { Injectable } from '@angular/core';
import { events, CurrentWindowDetails, HandleDescriptionPayload, HandleDimensionPayload, PickSlicesPayload,} from '@minsky/shared';
import { ipcRenderer } from 'electron';
import isElectron from 'is-electron';
import {Minsky, CppClass} from '@minsky/shared';
import {BrowserWindow} from 'electron';

@Injectable({
  providedIn: 'root',
})
export class ElectronService {
  ipcRenderer: typeof ipcRenderer;
  isElectron = isElectron? isElectron(): false;
  minsky: Minsky;
  
  constructor() {
    if (this.isElectron) {
      this.ipcRenderer = (<any>window).require('electron').ipcRenderer;
      this.minsky=new Minsky("/minsky");
      CppClass.backend=async (...args)=>{
        return await this.ipcRenderer.invoke(events.BACKEND, ...args);
      }
    }
  }

  async getCurrentWindow(): Promise<CurrentWindowDetails> {
    return this.ipcRenderer.invoke(events.GET_CURRENT_WINDOW);
  }
  
  async closeWindow(): Promise<void> {return this.ipcRenderer.invoke(events.CLOSE_WINDOW);}

  async openFileDialog(options): Promise<string> {
    return await this.ipcRenderer.invoke(events.OPEN_FILE_DIALOG, options);
  }
  
  async saveFileDialog(options): Promise<string> {
    return await this.ipcRenderer.invoke(events.SAVE_FILE_DIALOG, options);
  }
  
  async showMessageBoxSync(options): Promise<number> {
    return await this.ipcRenderer.invoke(events.SHOW_MESSAGE_BOX, options);
  }
  
  async saveHandleDescription(payload: HandleDescriptionPayload): Promise<number> {
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
