import { Injectable } from '@angular/core';
import { events, CurrentWindowDetails, HandleDescriptionPayload, HandleDimensionPayload, DownloadCSVPayload,} from '@minsky/shared';
import isElectron from 'is-electron';
import {Minsky, CppClass} from '@minsky/shared';

@Injectable({
  providedIn: 'root',
})
export class ElectronService {
  private ipcRenderer: any;
  public platform: string;
  isElectron = isElectron? isElectron(): false;
  minsky: Minsky;
  on: (channel: string, listener) => void;
  
  constructor() {
    this.minsky=new Minsky("minsky");

    if (this.isElectron) {
      this.ipcRenderer = window['electron'].ipcRenderer;
      this.platform = window['electron'].platform;
      this.on = window['electron'].ipcRendererOn;
      
      CppClass.backend=async (...args)=>{
        return await this.ipcRenderer.invoke(events.BACKEND, ...args);
      }
      CppClass.backendSync=async (...args)=>{
        return await this.ipcRenderer.invoke(events.BACKEND_SYNC, ...args);
      }
      CppClass.logMessage=async (message: string)=>{
        return await this.ipcRenderer.invoke(events.LOG_MESSAGE, message);
      }
    }
    else
      this.on = (...args)=>{};
  }

  send(channel: string,...args) {return this.ipcRenderer.send(channel,...args);}
  sendSync(channel: string,...args) {return this.ipcRenderer.sendSync(channel,...args);}
  invoke(channel: string,...args) {return this.ipcRenderer.invoke(channel,...args);}
  log(msg: string) {this.ipcRenderer.invoke(events.LOG,msg);}

  isWindows() {return this.platform === 'win32';}
  isMacOS() {return this.platform === 'darwin';}
  
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
  async downloadCSV(payload: DownloadCSVPayload): Promise<string> {
    return await this.ipcRenderer.invoke(events.DOWNLOAD_CSV, payload)
  }
  displayInitHelp() {
    this.ipcRenderer.invoke(events.DISPLAY_INIT_HELP);
  }
}
