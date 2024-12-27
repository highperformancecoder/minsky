import { BrowserWindow, Menu } from 'electron';

export interface ActiveWindow {
  id: number;
  size: number[];
  isMainWindow: boolean;
  context: BrowserWindow;
  systemWindowId: bigint;
  menu: Menu;
  url: string;
}
