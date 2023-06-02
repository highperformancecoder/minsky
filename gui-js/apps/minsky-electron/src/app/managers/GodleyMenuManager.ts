import {
  CanvasItem,
  ClassType,
  GodleyTableOutputStyles,
  Functions,
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR,
  minsky, GodleyIcon,
  GodleyTableWindow,
  events
} from '@minsky/shared';
import { Menu, MenuItem, IpcMainInvokeEvent } from 'electron';
import { CommandsManager } from './CommandsManager';
import { StoreManager } from './StoreManager';
import { WindowManager,} from './WindowManager';

export class GodleyMenuManager {
  public static async createMenusForGodleyView(
    window: Electron.BrowserWindow,
    itemInfo: CanvasItem
  ) {
    const scope = this;
    const godley = new GodleyIcon(minsky.namedItems.elem(itemInfo.id).second);
    const menu = Menu.buildFromTemplate([
      scope.getGodleyFileMenuItem(itemInfo, godley),
      // TODO remove itemInfo from this call
      scope.getGodleyEditMenuItem(itemInfo, godley),
      scope.getGodleyViewMenuItem(window, godley),
      scope.getGodleyOptionsMenuItem(itemInfo),
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

  private static refresh(id: string) {
    WindowManager.getWindowByUid(id).context?.webContents?.send(events.GODLEY_POPUP_REFRESH);
  }

  private static async setGodleyPreferences(
    property:
      | 'enableMultipleEquityColumns'
      | 'godleyTableShowValues'
      | 'godleyTableOutputStyle',
    value: boolean | GodleyTableOutputStyles,
    id: string
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
      GodleyMenuManager.refresh(id);
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

  private static getGodleyOptionsMenuItem(itemInfo: CanvasItem) {
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
                  itemInfo.id
                );
              },
            },
            {
              label: 'Disable',
              click: async () => {
                await scope.setGodleyPreferences(
                  'enableMultipleEquityColumns',
                  false,
                  itemInfo.id
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
    godley: GodleyIcon
  ) {
    return new MenuItem({
      label: 'View',
      submenu: [
        {
          label: 'Zoom In',
          accelerator: 'CmdOrCtrl + Plus',
          click: async () => {godley.popup.zoom(0,0,ZOOM_IN_FACTOR);}
        },
        {
          label: 'Zoom Out',
          accelerator: 'CmdOrCtrl + Minus',
          click: async () => {godley.popup.zoom(0,0,ZOOM_OUT_FACTOR);}
        },
        {
          label: 'Reset Zoom',
          click: async () => {
            godley.popup.zoomFactor(1);
            godley.popup.requestRedraw();
          },
        },
      ],
    });
  }

  private static getGodleyEditMenuItem(
    itemInfo: CanvasItem,
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
          click: () => {CommandsManager.editGodleyTitle(itemInfo.id);},
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

  private static getGodleyFileMenuItem(itemInfo: CanvasItem, godley: GodleyIcon) {
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
          click: () => {GodleyMenuManager.refresh(itemInfo.id);},
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

