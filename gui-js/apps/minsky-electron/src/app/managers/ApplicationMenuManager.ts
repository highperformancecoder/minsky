import {
  electronMenuBarHeightForWindows,
  Functions,
  importCSVvariableName,
  minsky,
} from '@minsky/shared';
import {
  dialog,
  Menu,
  MenuItem,
  MenuItemConstructorOptions,
  shell,
} from 'electron';
import { CommandsManager } from './CommandsManager';
import { StoreManager } from './StoreManager';
import { WindowManager } from './WindowManager';
import { BookmarkManager } from './BookmarkManager';
import { RecordingManager } from './RecordingManager';

//TODO:: Remove hardcoding of popup dimensions

export class ApplicationMenuManager {
  public static async createMainApplicationMenu() {
    const scope = this;
    const menu = Menu.buildFromTemplate([
      await scope.getFileMenu(),
      scope.getEditMenu(),
      scope.getBookmarksMenu(),
      scope.getInsertMenu(),
      scope.getOptionsMenu(),
      scope.getSimulationMenu(),
      scope.getHelpMenu(),
    ]);

    if (Functions.isMacOS())
      menu.insert(0, new MenuItem({
        label: "Minsky",
        submenu: [
        {
          label: 'About Minsky',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 420,
              height: 600,
              title: '',
              url: `#/headless/menu/file/about`,
              modal: false,
            });
            shell.beep();
          },
        },
        { type: 'separator' },
        {
          label: 'Preferences',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 500,
              useContentSize: true,
              height: 550+(Functions.isWindows()? electronMenuBarHeightForWindows:0),
              title: 'Preferences',
              url: `#/headless/menu/options/preferences`,
            });
          },
        },
          { role: 'services' },
          { type: 'separator' },
          { role: 'hide' },
          { role: 'hideOthers' },
          { role: 'unhide' },
          { type: 'separator' },
          { role: 'quit' }
        ]
    }));

    Menu.setApplicationMenu(menu);
    return menu;
  }

  private static async getFileMenu(): Promise<MenuItemConstructorOptions> {
    const scope = this;
    const ravelAvailable=await minsky.ravelAvailable();
    let upgradeLabel='Upgrade';
    if (isWindows() && !ravelAvailable)
      upgradeLabel+=' to Ravel';
    return {
      label: 'File',
      submenu: [
        {
          label: 'About',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 420,
              height: 500,
              title: '',
              url: `#/headless/menu/file/about`,
              modal: false,
            });
            shell.beep();
          },
        },
        {
          label: upgradeLabel,
          click() {CommandsManager.upgrade();},
        },
        {
          label: 'New System',
          accelerator: 'CmdOrCtrl + Shift + N',
          async click() {
              await CommandsManager.createNewSystem();
              BookmarkManager.updateBookmarkList();
          },
        },
        {
          label: 'Open',
          accelerator: 'CmdOrCtrl + O',
          enabled: true,
          async click() {
            try {
              const _dialog = await dialog.showOpenDialog({
                properties: ['openFile'],
                filters: [
                  { name: 'Minsky/Ravel', extensions: ['rvl','mky'] },
                  { name: '*.xml', extensions: ['xml'] },
                  { name: '*.*', extensions: ['*'] },
                ],
              });

              if (_dialog.canceled || !_dialog.filePaths) {
                return;
              }
              const filePath = _dialog.filePaths[0].toString();

              await CommandsManager.openNamedFile(filePath);
            } catch (error) {
              console.error(error);
            }
            BookmarkManager.updateBookmarkList();
          },
        },
        {
          label: 'Open Recent',
          id: 'openRecent',
          submenu: [
            { type: 'separator' },
            {
              label: 'Clear Recent',
              id: 'clearRecent',
              click: () => {
                StoreManager.store.set('recentFiles', []);
                Menu.getApplicationMenu()
                  .getMenuItemById('openRecent')
                  .submenu.items.forEach((i) => {
                    if (i.id !== 'clearRecent') {
                      i.visible = false;
                    }
                  });
              },
            },
          ],
        },
        {
          label: 'Library',
          click() {
            shell.openExternal(
              'https://github.com/highperformancecoder/minsky-models'
            );
          },
        },
        {
            label: 'Save',
            accelerator: 'CmdOrCtrl + S',
            async click() {await CommandsManager.save();}
        },
        {
            label: 'Save As',
            accelerator: 'CmdOrCtrl + Shift + S',
            async click() {await CommandsManager.saveAs();}
        },
        {
          label: 'Import Data',
          async click() {
            minsky.canvas.addVariable(importCSVvariableName, 'parameter');
            CommandsManager.importCSV(await CommandsManager.getFocusItemInfo(), true);
          }
        },
        {
          label: 'Insert File as Group',
          async click() {
            try {
              const insertGroupDialog = await dialog.showOpenDialog({
                properties: ['openFile'],
              });

              minsky.insertGroupFromFile(insertGroupDialog.filePaths[0].toString());
              await CommandsManager.requestRedraw();
            } catch (err) {
              console.error('file is not selected', err);
            }
          },
        },
        {
          label: 'Dimensional Analysis',
          click: async () => {
            const res = await minsky.dimensionalAnalysis();

            // empty object is returned if no error
            if (typeof res=="object") {
              dialog.showMessageBoxSync(WindowManager.getMainWindow(), {
                type: 'info',
                title: 'Dimensional Analysis',
                message: 'Dimensional Analysis Passed',
              });
            }
          },
        },
        scope.getExportCanvasMenu(),
        scope.getExportPlotMenu(),
        {
          label: 'Log simulation',
          id: 'logging-menu-item', // allows setting and clearing checkmark
          type: 'checkbox',
          async click() {
            if (!await minsky.loggingEnabled())
            {
              WindowManager.createPopupWindowWithRouting({
                width: 250,
                height: 600,
                title: 'Log simulation',
                url: `#/headless/menu/file/log-simulation`,
              });
            }
            else
            {
              minsky.closeLogFile();
              Menu.getApplicationMenu().getMenuItemById('logging-menu-item').checked=false;
            }
          },
        },
        {
          label: 'Recording',
          click() {RecordingManager.handleRecord();},
        },
        {
          label: 'Replay recording',
          click() {RecordingManager.handleRecordingReplay();},
        },
        {
          label: 'Quit',
          accelerator: 'CmdOrCtrl + Q',
          role: 'quit',
        },
        {
          type: 'separator',
        },
        {
          label: 'Debugging Use',
          enabled: false,
        },
        {
          label: 'Redraw',
          async click() {
                const {
                  leftOffset,
                  canvasWidth,
                  canvasHeight,
                  activeWindows,
                  electronTopOffset,
                  scaleFactor,
                } = WindowManager;

            minsky.canvas.renderFrame
            ({
              parentWindowId: activeWindows.get(1).systemWindowId.toString(),
              offsetLeft: leftOffset,
              offsetTop: electronTopOffset,
              childWidth: canvasWidth,
              childHeight: canvasHeight,
              scaleFactor: scaleFactor
            });
          },
        },
      ],
    };
  }

  private static getEditMenu(): MenuItemConstructorOptions {
    return {
      label: 'Edit',
      submenu: [
        {
          label: 'Undo',
          accelerator: 'CmdOrCtrl + Z',
          async click() {CommandsManager.undo(1);},
        },
        {
          label: 'Redo',
          accelerator: 'CmdOrCtrl + Y',
          async click() {CommandsManager.undo(-1);},
        },
        {
          label: 'Cut',
          accelerator: 'CmdOrCtrl + X',
          async click() {
            await CommandsManager.cut();
          },
        },
        {
          label: 'Copy',
          accelerator: 'CmdOrCtrl + C',
          async click() {
            await CommandsManager.copy();
          },
        },
        {
          label: 'Paste',
          accelerator: 'CmdOrCtrl + V',
          async click() {
            await CommandsManager.paste();
          },
        },
        {
          label: 'Group selection',
          async click() {minsky.canvas.groupSelection();},
        },
        {
          label: 'Dimensions',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 500,
              height: 500,
              minWidth: 500,
              title: 'Dimensions',
              url: `#/headless/menu/edit/dimensions`,
            });
          },
        },
        {
          label: 'Remove Units',
          async click() {minsky.deleteAllUnits();},
        },
        {
          label: 'Auto Layout',
          async click() {
            // TODO handle this asynchronously on the backend.
            minsky.autoLayout();
          },
        },
        {
          label: 'Random Layout',
          async click() {minsky.randomLayout();}
        },
      ],
    };
  }

  private static getInsertMenu(): MenuItemConstructorOptions {
    let varDialogParams={
      width: 400,
      height: 500,
      title: 'Create',
      url: '',
    };
    return {
      label: 'Insert',
      id: 'insert',
      submenu: [
        {
          label: 'Variable Browser',
          click() {
            WindowManager.createPopupWindowWithRouting({
              title: 'Variables',
              url: "#/headless/variable-pane",
              width: 400,
              height: 450,
              alwaysOnTop: true,
              modal: false,
            });
          },
        },
        {
          label: 'plot',
          async click() {minsky.canvas.addPlot();}
        },
        {
          label: 'sheet',
          async click() {minsky.canvas.addSheet();}
        },
        {
          label: 'Godley Table',
          async click() {minsky.canvas.addGodley();}
        },
        {
          label: 'Variable',
          submenu: [
            {
              type: 'separator',
            },
            {
              label: 'variable',
              click() {
                varDialogParams.url='#/headless/menu/insert/create-variable?type=flow';
                WindowManager.createPopupWindowWithRouting(varDialogParams);
              },
            },
            {
              label: 'constant',
              click() {
                varDialogParams.url='#/headless/menu/insert/create-variable?type=constant';
                WindowManager.createPopupWindowWithRouting(varDialogParams);
              },
            },
            {
              label: 'parameter',
              click() {
                varDialogParams.url='#/headless/menu/insert/create-variable?type=parameter';
                WindowManager.createPopupWindowWithRouting(varDialogParams);
              },
            },
          ],
        },
        {
          label: 'time',
          async click() {minsky.canvas.addOperation("time");}
        },
        {
          label: 'integrate',
          async click() {minsky.canvas.addOperation("integrate");}
        },
        {
          label: 'differentiate',
          async click() {minsky.canvas.addOperation("differentiate");},
        },
        {
          label: 'ravel',
          async click() {minsky.canvas.addOperation("ravel");}
        },
      ],
    };
  }

  private static getBookmarksMenu(): MenuItemConstructorOptions {
    return {
      label: 'Bookmarks',
      id: 'main-bookmark',
      submenu: [
        {
          label: 'Bookmark this position',
          click() {
            CommandsManager.bookmarkThisPosition();
          },
        },
        {
          label: 'Delete bookmark',
          id: 'delete-bookmark',
          submenu: [],
        },
        { type: 'separator' },
      ],
    };
  }

  private static async exportCanvas(
    extension: string, label: string,
    ...args: any[]
  ) {
    var filePath = await CommandsManager.getFilePathFromExportCanvasDialog(extension,label);
    if (filePath) {
      switch (extension) {
      case 'svg':
      case 'pdf':
      case 'emf':
      case 'ps':
      case 'eps':
      case 'png':
        CommandsManager.exportItemAsImage(WindowManager.currentTab,extension,filePath);
        break;
      case 'tex':
        minsky.latex(filePath,args[0]);
        break;
      case 'm':
        minsky.matlab(filePath);
        break;
      }
    }
  }

  private static getExportCanvasMenu(): MenuItemConstructorOptions {
    const scope = this;
    return {
      label: 'Export Canvas as',
      submenu: [
        {
          label: 'SVG',
          click: () => {scope.exportCanvas('svg','SVG');}
        },
        {
          label: 'PDF',
          click: () => {scope.exportCanvas('pdf','PDF');}
        },
        {
          label: 'EMF',
          visible: Functions.isWindows(),
          click: () => {scope.exportCanvas('emf','EMF');}
        },
        {
          label: 'PostScript',
          click: () => {scope.exportCanvas('eps','Postcsript');}
        },
        {
          label: 'Portable Network Graphics',
          click: () => {scope.exportCanvas('png','Portable Network Graphics');}
        },
        {
          label: 'LaTeX',
          click: () => {scope.exportCanvas(
            'tex','LaTeX',
            StoreManager.store.get('preferences').wrapLongEquationsInLatexExport,
          );},
        },
        {
          label: 'Matlab',
          click: () => {scope.exportCanvas('m','Matlab');}
        },
      ],
    };
  }

  private static async exportPlot(extension: string, command: (file:string)=>void) {
    const exportPlotDialog = await dialog.showSaveDialog({
      title: `Export plot as ${extension}`,
      defaultPath: 'plot',
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [{ extensions: [extension], name: extension.toUpperCase() }],
    });

    const { canceled, filePath } = exportPlotDialog;
    if (canceled || !filePath) {
      return;
    }
    command(filePath);
  }

  private static getExportPlotMenu(): MenuItemConstructorOptions {
    const scope = this;
    return {
      label: 'Export Plots as',
      submenu: [
        {
          label: 'SVG',
          async click() {
            await scope.exportPlot(
              'svg',
              (file:string)=>{minsky.renderAllPlotsAsSVG(file);}
            );
          },
        },
        {
          label: 'CSV',
          async click() {
            await scope.exportPlot(
              'csv',
              (file:string)=>{minsky.exportAllPlotsAsCSV(file);}
            );
          },
        },
      ],
    };
  }

  private static getOptionsMenu(): MenuItemConstructorOptions {
    return {
      label: 'Options',
      submenu: [
        {
          label: 'Preferences',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 500,
              useContentSize: true,
              height: 550+(Functions.isWindows()? electronMenuBarHeightForWindows:0),
              title: 'Preferences',
              url: `#/headless/menu/options/preferences`,
            });
          },
        },
        {
          label: 'Background Colour',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 450,
              height: 620,
              title: 'Background Colour',
              url: `#/headless/menu/options/background-color`,
            });
          },
        },
      ],
    };
  }

  private static getSimulationMenu(): MenuItemConstructorOptions {
    return {
      label: 'Simulation',
      submenu: [
        {
          label: 'Simulation',
          click() {
            WindowManager.createPopupWindowWithRouting({
              width: 550,
              height: 550,
              title: 'Simulation',
              url: `#/headless/menu/simulation/simulation-parameters`,
            });
          },
        },
      ],
    };
  }

  private static getHelpMenu(): MenuItemConstructorOptions {
    return {
      role: 'help',
      submenu: [
        {
          label: 'Ravel Documentation (F1)',
          click() {CommandsManager.loadHelpFile("Ravel");}
        },
      ],
    };
  }

  private static buildSubmenuForOperations(operations: string[]) {
    const submenu: MenuItemConstructorOptions[] = [];
    for (const o of operations) {
      submenu.push(this.addOpMenu(o));
    }
    return submenu;
  }

  private static addOpMenu(operation: string) {
    return {
      label: operation,
      async click() {minsky.canvas.addOperation(operation);}
    };
  }
  static async buildMenuForInsertOperations() {
    const availableOperationsMapping = await minsky.availableOperationsMapping();
    let insertOperationsMenu: MenuItem[] = [];
    let menuNames={
      "constop": "Fundamental Constants",
      "binop": "Binary Operations",
      "function": "Unary Functions",
      "reduction": "Reductions",
      "scan": "Scans",
      "tensor": "General Tensor Operations"
    };

    for (const key in availableOperationsMapping) {
      insertOperationsMenu = [
        ...insertOperationsMenu,
        new MenuItem({
          label: key in menuNames? menuNames[key]: key,
          submenu: this.buildSubmenuForOperations(
            availableOperationsMapping[key] as string[]
          ),
        }),
      ];
    }

    const insertMenu = Menu.getApplicationMenu().getMenuItemById('insert');

    insertOperationsMenu.forEach((o) => {
      insertMenu.submenu.append(o);
    });
  }
}
