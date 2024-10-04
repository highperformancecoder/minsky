import { contextBridge, ipcRenderer } from 'electron';

// Note exposing ipcRenderer in this way is considered bad practice (https://www.electronjs.org/docs/latest/tutorial/context-isolation#security-considerations)
// However, this issue is entirely mitigated because render code is trusted (part of this repo).
// Quite why this isn't the case generally with electron is a mystery to me.
contextBridge.exposeInMainWorld('electron', {
  getAppVersion: () => ipcRenderer.invoke('get-app-version'),
  ipcRendererOn: (channel: string, listener) => ipcRenderer.on(channel,listener),
  ipcRenderer: { ...ipcRenderer, send: ipcRenderer.send, sendSync: ipcRenderer.sendSync, invoke: ipcRenderer.invoke },
  platform: process.platform,
});
