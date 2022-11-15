import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('electron', {
  getAppVersion: () => ipcRenderer.invoke('get-app-version'),
  ipcRendererOn: (channel: string, listener) => ipcRenderer.on(channel,listener),
  platform: process.platform,
  ipcRenderer: ipcRenderer,
  platform: process.platform,
});
