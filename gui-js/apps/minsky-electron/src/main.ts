import { app, BrowserWindow } from 'electron';
import App from './app/app';
import ElectronEvents from './app/events/electron.events';

export default class Main {

  static bootstrapApp() {
    App.main(app, BrowserWindow);
  }

  static bootstrapAppEvents() {
    ElectronEvents.bootstrapElectronEvents();
   }
}

// bootstrap app
Main.bootstrapApp();
Main.bootstrapAppEvents();
