import {
  CanvasItem,
  ClassType,
  GodleyTableOutputStyles,
  Functions,
  minsky, GodleyIcon,
  GodleyTableWindow,
  events
} from '@minsky/shared';
import { Menu, MenuItem, IpcMainInvokeEvent } from 'electron';
import { CommandsManager } from './CommandsManager';
import { StoreManager } from './StoreManager';

export class GodleyMenuManager {
  public static async createMenusForGodleyView(
    window: Electron.BrowserWindow,
    itemInfo: CanvasItem
  ) {
    const scope = this;
    const godley = new GodleyIcon(minsky.namedItems.elem(itemInfo.id).second);
    const menu = Menu.buildFromTemplate([
      scope.getGodleyFileMenuItem(window, godley),
      scope.getGodleyEditMenuItem(godley),
      scope.getGodleyViewMenuItem(window),
      scope.getGodleyOptionsMenuItem(window),
      new MenuItem({
        label: 'Help',
        submenu: [
          {
            label: 'Help',
            click: () => {
              CommandsManager.loadHelpFile(ClassType.GodleyIcon);
            },
          },
        ],
      }),
    ]);
    if(Functions.isMacOS()) {
      Menu.setApplicationMenu(menu);
    } else {
      window.setMenu(menu);
    }
    return menu;
  }

  private static refresh(window: Electron.BrowserWindow) {
    window.webContents?.send(events.GODLEY_POPUP_REFRESH);
  }

  private static async setGodleyPreferences(
    property:
      | 'enableMultipleEquityColumns'
      | 'godleyTableShowValues'
      | 'godleyTableOutputStyle',
    value: boolean | GodleyTableOutputStyles,
    window: Electron.BrowserWindow,
  ) {
    const preferences = StoreManager.store.get('preferences');
    let {
      enableMultipleEquityColumns,
      godleyTableShowValues,
      godleyTableOutputStyle,
    } = preferences;

    if (property === 'enableMultipleEquityColumns') {
      enableMultipleEquityColumns = value as boolean;
      minsky.multipleEquities(enableMultipleEquityColumns);
      GodleyMenuManager.refresh(window);
    } else {
      if (property === 'godleyTableOutputStyle') {
        godleyTableOutputStyle = value as GodleyTableOutputStyles;
      } else if (property === 'godleyTableShowValues') {
        godleyTableShowValues = value as boolean;
      }
      minsky.setGodleyDisplayValue(godleyTableShowValues, godleyTableOutputStyle);
    }

    StoreManager.store.set({
      preferences: {
        ...preferences,
        godleyTableShowValues: godleyTableShowValues,
        godleyTableOutputStyle: godleyTableOutputStyle,
        enableMultipleEquityColumns: enableMultipleEquityColumns,
      },
    });
  }

  private static getGodleyOptionsMenuItem(window: Electron.BrowserWindow) {
    const scope = this;
    // CAVEAT:: Electron does not support dynamic menu labels  https://github.com/electron/electron/issues/5055)
    // Recreating menus from scratch leads to glitches after few clicks. Hence we have added submenus instead of providing toggle options / checkboxes

    return new MenuItem({
      label: 'Options',
      submenu: [
// disabling values and DR/CR style, as this functionality is not currently supported in the HTML Godley popup.
        //        {
//          label: 'Values',
//          submenu: [
//            {
//              label: 'Show',
//              click: async () => {
//                await scope.setGodleyPreferences('godleyTableShowValues', true, itemInfo.id);
//              },
//            },
//            {
//              label: 'Hide',
//              click: async () => {
//                await scope.setGodleyPreferences(
//                  'godleyTableShowValues',
//                  false,
//                  itemInfo.id
//                );
//              },
//            },
//          ],
//        },
//        {
//          label: 'DR/CR Style',
//          submenu: [
//            {
//              label: 'Sign',
//              click: async () => {
//                await scope.setGodleyPreferences(
//                  'godleyTableOutputStyle',
//                  GodleyTableOutputStyles.SIGN,
//                  itemInfo.id
//                );
//              },
//            },
//            {
//              label: 'DR/CR',
//              click: async () => {
//                await scope.setGodleyPreferences(
//                  'godleyTableOutputStyle',
//                  GodleyTableOutputStyles.DRCR,
//                  itemInfo.id
//                );
//              },
//            },
//          ],
//        },
        {
          label: 'Multiple Equity Column',
          submenu: [
            {
              label: 'Enable',
              click: async () => {
                await scope.setGodleyPreferences(
                  'enableMultipleEquityColumns',
                  true,
                  window
                );
              },
            },
            {
              label: 'Disable',
              click: async () => {
                await scope.setGodleyPreferences(
                  'enableMultipleEquityColumns',
                  false,
                  window
                );
              },
            },
          ],
        },
      ],
    });
  }

  private static getGodleyViewMenuItem(
    window: Electron.BrowserWindow,
  ) {
    return new MenuItem({
      label: 'View',
      submenu: [
        {
          label: 'Zoom In',
          accelerator: 'CmdOrCtrl + Plus',
          click: async () => {window.webContents?.send(events.ZOOM, 1.1);}
        },
        {
          label: 'Zoom Out',
          accelerator: 'CmdOrCtrl + Minus',
          click: async () => {window.webContents?.send(events.ZOOM, 1.0/1.1);}
        },
        {
          label: 'Reset Zoom',
          click: async () => {window.webContents?.send(events.RESET_ZOOM);}
        },
      ],
    });
  }

  private static getGodleyEditMenuItem(
    godley: GodleyIcon
  ) {
    return new MenuItem({
      label: 'Edit',
      submenu: [
        {
          label: 'Undo',
          accelerator: 'CmdOrCtrl + z',
          click: () => {godley.popup.undo(1);},
        },
        {
          label: 'Redo',
          accelerator: 'CmdOrCtrl + y',
          click: () => {godley.popup.undo(-1);},
        },
        {
          label: 'Title',
          click: () => {CommandsManager.editGodleyTitle(godley);},
        },
        {
          label: 'Cut',
          accelerator: 'CmdOrCtrl + x',
          click: () => {godley.popup.cut();},
        },
        {
          label: 'Copy',
          accelerator: 'CmdOrCtrl + c',
          click: () => {godley.popup.copy();}
        },
        {
          label: 'Paste',
          accelerator: 'CmdOrCtrl + v',
          click: () => {godley.popup.paste();}
        },
      ],
    });
  }

  private static getGodleyFileMenuItem(window: Electron.BrowserWindow, godley: GodleyIcon) {
    return new MenuItem({
      label: 'File',
      submenu: [
        {
          label: 'Export as',
          submenu: [
            {
              label: 'CSV',
              click: async () => {
                const command = godley.table.exportToCSV;
                await CommandsManager.exportGodleyAs('csv', command);
              },
            },
            {
              label: 'LaTeX',
              click: async () => {
                const command = godley.table.exportToLaTeX;
                await CommandsManager.exportGodleyAs('tex', command);
              },
            },
          ],
        },
        {
          label: 'Refresh',
          click: () => {GodleyMenuManager.refresh(window);},
        }
      ],
    });
  }

  /// handle mouse down events in a Godley view
  static async mouseDown(window: GodleyTableWindow, x: number, y: number) {
    var clickType=window.clickTypeZoomed(x,y);
    if (clickType==="importStock")
    {
      const c = window.colXZoomed(x);
      this.importStock(window, c);
    } else {window.mouseDown(x,y);}
  }

  static async importStock(window: GodleyTableWindow, c, event?: IpcMainInvokeEvent) {
    var importOptions=window.matchingTableColumnsByCol(c);
      var menu=new Menu();
      for (var v in importOptions) 
        menu.append(new MenuItem({
          label: importOptions[v],
          click: (item) => {
            window.importStockVarByCol(item.label, c);
            if(event) event.sender.send(events.GODLEY_POPUP_REFRESH);
          }
        }));
      menu.popup();
  }
}

