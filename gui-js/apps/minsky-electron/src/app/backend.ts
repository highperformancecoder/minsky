import {backend, setErrorDialog, restService} from './backend/backend';
import {GodleyIcon} from './backend/godleyIcon';
import {GodleyTableWindow} from './backend/godleyTableWindow';
import {Group} from './backend/group';
import {Item} from './backend/item';
import {minsky} from './backend/minsky';
import {PlotWidget} from './backend/plot'
import {Operation} from './backend/operation'
import {Ravel} from './backend/ravel'
import {RenderNativeWindow} from './backend/renderNativeWindow';
import {Variable} from './backend/variable'
import { Utility } from './utility';
import { WindowManager } from './managers/WindowManager';
import { dialog, ipcMain, shell } from 'electron';
import { version } from '@minsky/shared';

export {
  restService, minsky, GodleyIcon, GodleyTableWindow, Group, Item, PlotWidget, Operation,
  Ravel, RenderNativeWindow, Variable
};

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

setErrorDialog((message: string)=>{
  dialog.showMessageBoxSync(WindowManager.getMainWindow(),{
    message: message,
    type: 'error',
  });
});

// Sanity checks before we get started
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

