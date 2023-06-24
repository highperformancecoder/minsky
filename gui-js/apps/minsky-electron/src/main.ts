import { app, BrowserWindow } from 'electron';
import App from './app/app';
import ElectronEvents from './app/events/electron.events';
import { Utility } from '@minsky/shared';

export default class Main {
  static initialize() {
  }

  static bootstrapApp() {
    App.main(app, BrowserWindow);
  }

  static bootstrapAppEvents() {
    ElectronEvents.bootstrapElectronEvents();

    // initialize auto updater service
    if (!Utility.isDevelopmentMode()) {
      // UpdateEvents.initAutoUpdateService();
    }
  }
}

// handle setup events as quickly as possible
Main.initialize();

// bootstrap app
Main.bootstrapApp();
Main.bootstrapAppEvents();
