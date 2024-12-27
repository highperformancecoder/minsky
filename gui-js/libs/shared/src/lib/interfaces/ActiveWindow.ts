import { BrowserWindow, Menu } from 'electron';

export interface ActiveWindow {
  id: number;
  size: number[];
  isMainWindow: boolean;
  context: BrowserWindow;
  systemWindowId: bigint;
  menu: Menu;
  url: string; /// url opened in this window, if any. Can be none or empty.
}
