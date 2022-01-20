import {
  CanvasItem,
  ClassType,
  commandsMapping,
  GodleyTableOutputStyles,
  green,
  isMacOS,
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR
} from '@minsky/shared';
import { Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { RestServiceManager } from './RestServiceManager';
import { StoreManager } from './StoreManager';

export class GodleyMenuManager {
  public static createMenusForGodleyView(
    window: Electron.BrowserWindow,
    itemInfo: CanvasItem
  ) {
    const scope = this;
    const itemAccessor = `${commandsMapping.GET_NAMED_ITEM}/${itemInfo.id}/second`;
    const menu = Menu.buildFromTemplate([
      scope.getGodleyFileMenuItem(itemAccessor),
      scope.getGodleyEditMenuItem(itemInfo, itemAccessor),
      scope.getGodleyViewMenuItem(window, itemInfo, itemAccessor),
      scope.getGodleyOptionsMenuItem(itemAccessor),
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
    if(isMacOS()) {
      Menu.setApplicationMenu(menu);
    } else {
      window.setMenu(menu);
    }
    return menu;
  }

  private static async setGodleyPreferences(
    property:
      | 'enableMultipleEquityColumns'
      | 'godleyTableShowValues'
      | 'godleyTableOutputStyle',
    value: boolean | GodleyTableOutputStyles
  ) {
    const preferences = StoreManager.store.get('preferences');
    let {
      enableMultipleEquityColumns,
      godleyTableShowValues,
      godleyTableOutputStyle,
    } = preferences;

    if (property === 'enableMultipleEquityColumns') {
      enableMultipleEquityColumns = value as boolean;
      await RestServiceManager.handleMinskyProcess({
        command: `${commandsMapping.MULTIPLE_EQUITIES} ${enableMultipleEquityColumns}`,
      });
    } else {
      if (property === 'godleyTableOutputStyle') {
        godleyTableOutputStyle = value as GodleyTableOutputStyles;
      } else if (property === 'godleyTableShowValues') {
        godleyTableShowValues = value as boolean;
      }
      await RestServiceManager.handleMinskyProcess({
        command: `${commandsMapping.SET_GODLEY_DISPLAY_VALUE} [${godleyTableShowValues},"${godleyTableOutputStyle}"]`,
      });
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

  private static getGodleyOptionsMenuItem(itemAccessor: string) {
    const scope = this;
    // CAVEAT:: Electron does not support dynamic menu labels  https://github.com/electron/electron/issues/5055)
    // Recreating menus from scratch leads to glitches after few clicks. Hence we have added submenus instead of providing toggle options / checkboxes

    return new MenuItem({
      label: 'Options',
      submenu: [
        {
          label: 'Values',
          submenu: [
            {
              label: 'Show',
              click: async () => {
                await scope.setGodleyPreferences('godleyTableShowValues', true);
              },
            },
            {
              label: 'Hide',
              click: async () => {
                await scope.setGodleyPreferences(
                  'godleyTableShowValues',
                  false
                );
              },
            },
          ],
        },
        {
          label: 'DR/CR Style',
          submenu: [
            {
              label: 'Sign',
              click: async () => {
                await scope.setGodleyPreferences(
                  'godleyTableOutputStyle',
                  GodleyTableOutputStyles.SIGN
                );
              },
            },
            {
              label: 'DR/CR',
              click: async () => {
                await scope.setGodleyPreferences(
                  'godleyTableOutputStyle',
                  GodleyTableOutputStyles.DRCR
                );
              },
            },
          ],
        },
        {
          label: 'Multiple Equity Column',
          submenu: [
            {
              label: 'Enable',
              click: async () => {
                await scope.setGodleyPreferences(
                  'enableMultipleEquityColumns',
                  true
                );
              },
            },
            {
              label: 'Disable',
              click: async () => {
                await scope.setGodleyPreferences(
                  'enableMultipleEquityColumns',
                  false
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
    itemInfo: CanvasItem,
    itemAccessor: string
  ) {
    return new MenuItem({
      label: 'View',
      submenu: [
        {
          label: 'Zoom In',
          accelerator: 'CmdOrCtrl + Plus',
          click: async () => {
            const [x, y] = window.getContentSize();

            await RestServiceManager.handleMinskyProcess({
              command: `${commandsMapping.GET_NAMED_ITEM}/${
                itemInfo.id
              }/second/popup/zoom [${x / 2},${y / 2},${ZOOM_IN_FACTOR}]`,
            });
          },
        },
        {
          label: 'Zoom Out',
          accelerator: 'CmdOrCtrl + Minus',
          click: async () => {
            const [x, y] = window.getContentSize();
            await RestServiceManager.handleMinskyProcess({
              command: `${commandsMapping.GET_NAMED_ITEM}/${
                itemInfo.id
              }/second/popup/zoom [${x / 2},${y / 2},${ZOOM_OUT_FACTOR}]`,
            });
          },
        },
        {
          label: 'Reset Zoom',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/zoomFactor 1`,
            });
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/requestRedraw`,
            });
          },
        },
      ],
    });
  }

  private static getGodleyEditMenuItem(
    itemInfo: CanvasItem,
    itemAccessor: string
  ) {
    return new MenuItem({
      label: 'Edit',
      submenu: [
        {
          label: 'Undo',
          accelerator: 'CmdOrCtrl + z',
          click: async () => {
            const numberOfTimes = 1;
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/undo ${numberOfTimes}`,
            });
          },
        },
        {
          label: 'Redo',
          accelerator: 'CmdOrCtrl + y',
          click: async () => {
            const numberOfTimes = -1;
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/undo ${numberOfTimes}`,
            });
          },
        },
        {
          label: 'Title',
          click: () => {
            CommandsManager.editGodleyTitle(itemInfo.id);
          },
        },
        {
          label: 'Cut',
          accelerator: 'CmdOrCtrl + x',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/cut`,
            });
            console.log(green('here'));
          },
        },
        {
          label: 'Copy',
          accelerator: 'CmdOrCtrl + c',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/copy`,
            });
          },
        },
        {
          label: 'Paste',
          accelerator: 'CmdOrCtrl + v',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: `${itemAccessor}/popup/paste`,
            });
          },
        },
      ],
    });
  }

  private static getGodleyFileMenuItem(itemAccessor: string) {
    return new MenuItem({
      label: 'File',
      submenu: [
        {
          label: 'Export as',
          submenu: [
            {
              label: 'CSV',
              click: async () => {
                const command = `${itemAccessor}/table/exportToCSV`;

                await CommandsManager.exportGodleyAs('csv', command);
              },
            },
            {
              label: 'LaTeX',
              click: async () => {
                const command = `${itemAccessor}/table/exportToLaTeX`;

                await CommandsManager.exportGodleyAs('tex', command);
              },
            },
          ],
        },
      ],
    });
  }
}
