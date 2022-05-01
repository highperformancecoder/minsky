import { Injectable } from '@angular/core';
import { events, MinskyProcessPayload } from '@minsky/shared';
import { ipcRenderer, remote } from 'electron';
import isElectron from 'is-electron';

@Injectable({
  providedIn: 'root',
})
export class ElectronService {
  ipcRenderer: typeof ipcRenderer;
  remote: typeof remote;
  isElectron = isElectron();

  constructor() {
    if (this.isElectron) {
      this.ipcRenderer = (<any>window).require('electron').ipcRenderer;
      this.remote = (<any>window).require('electron').remote;
    }
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
