import {
  CanvasItem,
  ClassType,
  isEmptyObject,
  isWindows,
  MainRenderingTabs,
} from '@minsky/shared';
import {minsky, Item, GodleyTableWindow} from '../backend';
import { BrowserWindow, Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { RestServiceManager } from './RestServiceManager';
import { WindowManager } from './WindowManager';
import * as log from 'electron-log';

export class ContextMenuManager {
  private static x: number = null;
  private static y: number = null;

  private static showAllPlotsOnTabChecked = false;
  
  public static async initContextMenu(x: number, y: number, type: string, command: string) {
    switch (type)
    {
      case "canvas":
      {
        const mainWindow = WindowManager.getMainWindow();
        this.x = x;
        this.y = y;
        
        const currentTab = RestServiceManager.getCurrentTab();
        
        switch (currentTab) {
        case MainRenderingTabs.canvas:
          await this.initContextMenuForWiring(mainWindow);
          break;
          
        case MainRenderingTabs.variables:
          await this.initContextMenuForVariableTab(mainWindow);
          break;
          
        case MainRenderingTabs.plot:
          await this.initContextMenuForPlotTab(mainWindow);
          break;
          
        default:
          break;
        }
      }
      break;
      case "godley":
      this.initContextMenuForGodleyPopup(command,x,y);
      break;
      default:
      log.warn("Unknown context menu for ",type);
      break;
    }
  }

  private static async initContextMenuForVariableTab(
    mainWindow: BrowserWindow
  ) {
    const rowNumber = minsky.variableTab.rowY(this.y);

    const clickType = minsky.variableTab.clickType(this.x, this.y);

    if (clickType === `internal`) {
      const varName = minsky.variableTab.getVarName(rowNumber);

      const menuItems = [
        new MenuItem({
          label: `Remove ${varName} from tab`,
          click:  () => {
            minsky.variableTab.toggleVarDisplay(rowNumber);
            minsky.variableTab.requestRedraw();
          },
        }),
      ];

      ContextMenuManager.buildAndDisplayContextMenu(menuItems, mainWindow);

      return;
    }
  }

  private static async initContextMenuForPlotTab(mainWindow: BrowserWindow) {
    minsky.plotTab.getItemAt(this.x,this.y)
    if (minsky.plotTab.item.properties()) {
      const menuItems = [
        new MenuItem({
          label: `Remove plot from tab`,
          click: async () => {
            minsky.plotTab.togglePlotDisplay();
            minsky.plotTab.requestRedraw();
          },
        }),
      ];

      ContextMenuManager.buildAndDisplayContextMenu(menuItems, mainWindow);

      return;
    }
  }

  private static async initContextMenuForWiring(mainWindow: BrowserWindow) {
    try {
      const wire = await CommandsManager.getWireAt(this.x, this.y);

      const isWirePresent = !isEmptyObject(wire);

      const isWireVisible = minsky.canvas.wire.visible();
      const itemInfo = await CommandsManager.getItemInfo(this.x, this.y);

        if (isWirePresent && isWireVisible && (itemInfo?.classType!=ClassType.Group||itemInfo?.displayContents)) {
        ContextMenuManager.buildAndDisplayContextMenu(
          ContextMenuManager.wireContextMenu(),
          mainWindow
        );
        return;
      }


      if (itemInfo?.classType) {
        switch (itemInfo?.classType) {
          case ClassType.GodleyIcon:
            ContextMenuManager.buildAndDisplayContextMenu(
              await ContextMenuManager.rightMouseGodley(itemInfo),
              mainWindow
            );
            break;

          case ClassType.Group:
            ContextMenuManager.buildAndDisplayContextMenu(
              await ContextMenuManager.rightMouseGroup(itemInfo),
              mainWindow
            );
            break;

          default:
            ContextMenuManager.buildAndDisplayContextMenu(
              await ContextMenuManager.contextMenu(itemInfo),
              mainWindow
            );

            break;
        }

        return;
      }
      
      ContextMenuManager.buildAndDisplayContextMenu(
        ContextMenuManager.canvasContext(),
        mainWindow
      );

      return;
    } catch (error) {
      console.error(
        '🚀 ~ file: contextMenuManager.ts ~ line 117 ~ ContextMenuManager ~ mainWindow.webContents.on ~ error',
        error
      );
    }
  }
  private static async rightMouseGodley(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    if (await CommandsManager.selectVar(this.x, this.y)) {
      const menuItems: MenuItem[] = [
        new MenuItem({
          label: 'Copy',
          click: () => {minsky.canvas.copyItem();}
        }),
        new MenuItem({
          label: 'Rename all instances',
          click: async () => {
            await CommandsManager.renameAllInstances(itemInfo);
          },
        }),
      ];

      return menuItems;
    } else {
      return await ContextMenuManager.contextMenu(itemInfo);
    }
  }

  private static async rightMouseGroup(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    if (await CommandsManager.selectVar(this.x, this.y)) {
      const menuItems = [
        new MenuItem({
          label: 'Edit',
          click: async () => {
            await CommandsManager.editItem(itemInfo.classType);
          },
        }),
        new MenuItem({
          label: 'Copy',
          click: () => {minsky.canvas.copyItem();}
        }),
        new MenuItem({
          label: 'Remove',
          click: () => {minsky.canvas.removeItemFromItsGroup();}
        }),
      ];

      return menuItems;
    } else {
      return await ContextMenuManager.contextMenu(itemInfo);
    }
  }

  private static wireContextMenu(): MenuItem[] {
    const menuItems = [
      new MenuItem({
        label: 'Description',
        click: () => {CommandsManager.postNote('wire');}
      }),
      new MenuItem({
        label: 'Straighten',
        click: () => {minsky.canvas.wire.straighten();}
      }),
      new MenuItem({
        label: 'Delete wire',
        click: () => {minsky.canvas.deleteWire();}
      }),
    ];

    return menuItems;
  }

  private static canvasContext(): MenuItem[] {
    const menuItems = [
      new MenuItem({
        label: 'Cut',
        click: () => {minsky.cut();}
      }),
      new MenuItem({
        label: 'Copy selection',
        click: () => {minsky.copy();}
      }),
      new MenuItem({
        label: 'Save selection as',
        click: async () => {
          await CommandsManager.saveSelectionAsFile();
        },
      }),
      new MenuItem({
        label: 'Paste selection',
        click: () => {
          CommandsManager.pasteAt(this.x, this.y);
        },
      }),
      new MenuItem({
        label: 'Hide defining groups of selected variables',
        click: async () => {minsky.canvas.pushDefiningVarsToTab();
          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Show all defining groups on canvas',
        click: async () => {
          minsky.canvas.showDefiningVarsOnCanvas();
          await CommandsManager.requestRedraw();
        },
      }),

      new MenuItem({
        label: 'Show all plots on tab',
        type: 'checkbox',
        checked: this.showAllPlotsOnTabChecked,
        click: async () => {
          minsky.canvas.showAllPlotsOnTab();
          this.showAllPlotsOnTabChecked = !this.showAllPlotsOnTabChecked;
        },
      }),
      new MenuItem({
        label: 'Bookmark here',
        click: async () => {
          await CommandsManager.bookmarkAt(this.x, this.y);
        },
      }),
      new MenuItem({
        label: 'Group',
        click: () => {minsky.canvas.groupSelection();}
      }),
      new MenuItem({
        label: 'Lock selected Ravels',
        click: async () => {minsky.canvas.lockRavelsInSelection();}
      }),
      new MenuItem({
        label: 'Unlock selected Ravels',
        click: async () => {minsky.canvas.unlockRavelsInSelection();}
      }),
      new MenuItem({
        label: 'Open master group',
        click: () => {minsky.openModelInCanvas();}
      }),
          ];

    return menuItems;
  }

  private static buildAndDisplayContextMenu(
    menuItems: MenuItem[],
    mainWindow: Electron.BrowserWindow
  ) {
    if (menuItems.length) {
      const menu = Menu.buildFromTemplate([
        new MenuItem({
          label: 'Help',
          visible:
            RestServiceManager.getCurrentTab() === MainRenderingTabs.canvas,
          click: async () => {
            await CommandsManager.help(this.x, this.y);
          },
        }),
        ...menuItems,
      ]);

      menu.popup({
        window: mainWindow,
        x: this.x,
        y: this.y + WindowManager.electronTopOffset,
      });
    }
  }

  private static async contextMenu(itemInfo: CanvasItem) {
    let menuItems: MenuItem[] = [
      new MenuItem({
        label: 'Description',
        click: () => {
          CommandsManager.postNote('item');
        },
      }),
    ];

    switch (itemInfo?.classType) {
      case ClassType.Variable:
      case ClassType.VarConstant:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForVariables(itemInfo)),
        ];
        break;

      case ClassType.Operation:
      case ClassType.IntOp:
      case ClassType.DataOp:
      case ClassType.UserFunction:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForOperations(itemInfo)),
        ];
        break;

      case ClassType.PlotWidget:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForPlotWidget(itemInfo)),
        ];
        break;

      case ClassType.GodleyIcon:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForGodleyIcon(itemInfo)),
        ];

        break;

      case ClassType.Group:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForGroup()),
        ];
        break;

      case ClassType.Item:
        menuItems = [
          ...menuItems,
          new MenuItem({
            label: 'Copy item',
            click: async () => {minsky.canvas.copyItem();}
          }),
        ];

        break;

      case ClassType.SwitchIcon:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForSwitchIcon()),
        ];

        break;

      case ClassType.Ravel:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForRavel()),
        ];

        break;

      case ClassType.Lock:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForLock()),
        ];

        break;

      default:
        break;
    }

    menuItems = [
      ...menuItems,
      new MenuItem({
        label: `Delete ${itemInfo.classType}`,
        click: async () => {
          await CommandsManager.deleteCurrentItemHavingId(itemInfo.id);
        },
      }),
    ];

    return menuItems;
  }

  private static async buildContextMenuForPlotWidget(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    const displayPlotOnTabChecked = minsky.canvas.item.plotTabDisplay();

    const menuItems = [
      new MenuItem({
        label: 'Expand',
        click: async () => {
          await CommandsManager.expandPlot(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Make Group Plot',
        click: async () => {minsky.canvas.item.makeDisplayPlot();}
      }),
      new MenuItem({
        label: 'Options',
        click: async () => {
          await CommandsManager.openPlotWindowOptions(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Pen Styles',
        click: () => {
          WindowManager.createPopupWindowWithRouting({
            title: `Pen Styles`,
            url: `#/headless/menu/context-menu/pen-styles`,
            height: 500,
            width: 350,
          });
        },
      }),
      new MenuItem({
        label: 'Display plot on tab',
        type: 'checkbox',
        checked: displayPlotOnTabChecked,
        click: async () => {minsky.canvas.item.togglePlotTabDisplay();}
      }),
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV();
        },
      }),
      new MenuItem({
        label: 'Export Image As',
        submenu: [
          {
            label: 'SVG',
            click: async () => {
              CommandsManager.exportItemAsImage('svg', 'SVG');
            },
          },
          {
            label: 'PDF',
            click: async () => {
              CommandsManager.exportItemAsImage('pdf', 'PDF');
            },
          },
          {
            label: 'PostScript',
            click: async () => {
              CommandsManager.exportItemAsImage('ps', 'PostScript');
            },
          },
          {
            label: 'EMF',
            visible: isWindows(),
            click: async () => {
              CommandsManager.exportItemAsImage('emf', 'EMF');
            },
          },
        ],
      }),
    ];

    return menuItems;
  }

  private static async buildContextMenuForGodleyIcon(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    const displayVariableChecked = minsky.canvas.item.variableDisplay();
    const rowColButtonsChecked = minsky.canvas.item.buttonDisplay();
    const editorModeChecked = minsky.canvas.item.editorMode();

    const menuItems = [
      new MenuItem({
        label: 'Open Godley Table',
        click: async () => {
          await CommandsManager.openGodleyTable(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Title',
        click: () => {
          CommandsManager.editGodleyTitle();
        },
      }),
      new MenuItem({
        label: 'Set currency',
        click: () => {
          CommandsManager.setGodleyCurrency();
        },
      }),
      new MenuItem({
        label: 'Editor mode',
        checked: editorModeChecked,
        type: 'checkbox',
        click: async () => {minsky.canvas.item.toggleEditorMode();}
      }),
      new MenuItem({
        label: 'Row/Col buttons',
        checked: rowColButtonsChecked,
        type: 'checkbox',
        click: async () => {minsky.canvas.item.toggleButtons();}
      }),
      new MenuItem({
        label: 'Display variables',
        type: 'checkbox',
        checked: displayVariableChecked,
        click: async () => {minsky.canvas.item.toggleVariableDisplay();}
      }),
      new MenuItem({
        label: 'Copy flow variables',
        click: async () => {minsky.canvas.copyAllFlowVars();}
      }),
      new MenuItem({
        label: 'Copy stock variables',
        click: async () => {minsky.canvas.copyAllStockVars();}
      }),
      new MenuItem({
        label: 'Export as',
        submenu: [
          {
            label: 'CSV',
            click: async () => {
              await CommandsManager.exportGodleyAs('csv');
            },
          },
          {
            label: 'LaTeX',
            click: async () => {
              await CommandsManager.exportGodleyAs('tex');
            },
          },
        ],
      }),
    ];
    return menuItems;
  }

  private static async buildContextMenuForOperations(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    let portValues = 'unknown';

    try {
      portValues = minsky.canvas.item.portValues();
    } catch (error) {
      portValues = 'unknown';
    }

    let menuItems = [
      new MenuItem({ label: `Port values ${portValues}}` }),
      new MenuItem({
        label: 'Edit',
        click: async () => {
          await CommandsManager.editItem(itemInfo.classType);
        },
      }),
    ];

    // TODO data is obsolete
    if ((await CommandsManager.getItemType()) === 'data') {
      menuItems.push(
        new MenuItem({
          label: 'Import Data',
          click: async () => {
            const filePath = await CommandsManager.getFilePathUsingSaveDialog();
            if (filePath) {minsky.canvas.item.readData(filePath);}
          }
        })
      );
    }

    menuItems = [
      ...menuItems,
      new MenuItem({
        label: 'Copy item',
        click: async () => {minsky.canvas.copyItem();}
      }),
      new MenuItem({
        label: 'Flip',
        click: async () => {
          await CommandsManager.flip();
        },
      }),
    ];

    if ((await CommandsManager.getItemType()) === 'integrate') {
      menuItems.push(
        new MenuItem({
          label: 'Toggle var binding',
          click: async () => {
            minsky.canvas.item.toggleCoupled();
            CommandsManager.requestRedraw();
          },
        })
      );
      menuItems.push(
        new MenuItem({
          label: 'Select all instances',
          click: () => {minsky.canvas.selectAllVariables();}
        })
      );
      menuItems.push(
        new MenuItem({
          label: 'Rename all instances',
          click: async () => {
            await CommandsManager.renameAllInstances(itemInfo);
          },
        })
      );
    }
    return menuItems;
  }

  private static async buildContextMenuForGroup(): Promise<MenuItem[]> {
    const menuItems = [
      new MenuItem({
        label: 'Edit',
        click: async () => {
          await CommandsManager.editItem(ClassType.Group);
        },
      }),
      new MenuItem({
        label: 'Open in canvas',
        click: async () => {minsky.openGroupInCanvas();}
      }),
      new MenuItem({
        label: 'Zoom to display',
        click: async () => {minsky.canvas.zoomToDisplay();
          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Remove plot icon',
        click: async () => {
          minsky.canvas.item.removeDisplayPlot();
          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Copy',
        click: () => {minsky.canvas.copyItem();}
      }),
      new MenuItem({
        label: 'Make subroutine',
        click: () => {minsky.canvas.item.makeSubroutine();}
      }),
      new MenuItem({
        label: 'Save group as',
        click: async () => {
          await CommandsManager.saveGroupAsFile();
        },
      }),
      new MenuItem({
        label: 'Flip',
        click: async () => {
          await CommandsManager.flip();
        },
      }),
      new MenuItem({
        label: 'Flip Contents',
        click: async () => {
          minsky.canvas.item.flipContents();
          CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Ungroup',
        click: async () => {
          minsky.canvas.ungroupItem();
          CommandsManager.requestRedraw();
        },
      }),
    ];

    return menuItems;
  }

  private static async buildContextMenuForRavel(): Promise<MenuItem[]> {
    let menuItems = [
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV();
        },
      }),
    ];

    if ((await CommandsManager.getLockGroup()).length) {
      menuItems = [
        ...menuItems,
        new MenuItem({ label: 'Lock specific handles' }),
        new MenuItem({ label: 'Axis properties' }),
        new MenuItem({
          label: 'Unlock',
          click: async () => {
            minsky.canvas.item.leaveLockGroup();
            CommandsManager.requestRedraw();
          },
        }),
      ];
    }

    return menuItems;
  }

  private static async buildContextMenuForSwitchIcon(): Promise<MenuItem[]> {
    const menuItems = [
      new MenuItem({
        label: 'Add case',
        click: async () => {
          await CommandsManager.incrCase(1);
        },
      }),
      new MenuItem({
        label: 'Delete case',
        click: async () => {
          await CommandsManager.incrCase(-1);
        },
      }),
      new MenuItem({
        label: 'Flip',
        click: async () => {
          await CommandsManager.flipSwitch();
        },
      }),
    ];

    return menuItems;
  }

  private static async buildContextMenuForLock(): Promise<MenuItem[]> {
    const isItemLocked = await CommandsManager.isItemLocked();

    return [
      new MenuItem({
        label: isItemLocked? 'Unlock': 'Lock',
        click: async () => {minsky.canvas.item.toggleLocked();}
      })
    ];
  }

  private static async buildContextMenuForVariables(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    let dims = null;

    if (
      itemInfo?.classType === ClassType.Variable ||
      itemInfo?.classType === ClassType.VarConstant
    ) {
      dims = await CommandsManager.getItemDims();
    }

    const local = minsky.canvas.item.local();
    
    const menuItems = [
      dims && dims.length
        ? new MenuItem({ label: `Dims ${dims.toString()}` })
        : new MenuItem({ label: `Value ${itemInfo?.value || ''}` }),
      new MenuItem({
        label: "Local",
        type: 'checkbox',
        checked: local,
        click: async () => {minsky.canvas.item.toggleLocal();}
      }),
      new MenuItem({
        label: 'Find definition',
        click: async () => {
          await CommandsManager.findDefinition();
        },
      }),
      new MenuItem({
        label: 'Select all instances',
        click: async () => {minsky.canvas.selectAllVariables();}
      }),
      new MenuItem({
        label: 'Find all instances',
        click: async () => {
          await CommandsManager.findAllInstances();
        },
      }),
      new MenuItem({
        label: 'Rename all instances',
        click: async () => {
          await CommandsManager.renameAllInstances(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Edit',
        click: async () => {
          await CommandsManager.editVar();
        },
      }),
      new MenuItem({
        label: 'Copy item',
        click: () => {minsky.canvas.copyItem();}
      }),
      new MenuItem({
        label: 'Add integral',
        click: () => {minsky.addIntegral();}
      }),
    ];

    if (await CommandsManager.isItemDefined()) {
      menuItems.push(
        new MenuItem({
          label: 'Display variable on tab',
          click: () => {minsky.canvas.item.toggleVarTabDisplay();}
        })
      );
    }

    menuItems.push(
      new MenuItem({
        label: 'Flip',
        click: async () => {
          await CommandsManager.flip();
        },
      })
    );

    if ((await CommandsManager.getItemType()) === 'parameter') {
      menuItems.push(
        new MenuItem({
          label: 'Import CSV',
          click: () => {
            CommandsManager.importCSV(itemInfo);
          },
        })
      );
    }

    menuItems.push(
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV();
        },
      })
    );

    return menuItems;
  }

  private static async initContextMenuForGodleyPopup(namedItemSubCommand: string, x: number, y: number)
  {
    const namedItem=new Item(namedItemSubCommand);
    
    var menu=new Menu();
    menu.append(new MenuItem({
      label: "Help",
      click: async ()=> {await CommandsManager.loadHelpFile("GodleyTable");},
    }));
    
    var id=namedItem.id();
    menu.append(new MenuItem({
      label: "Title",
      click: async ()=> {await CommandsManager.editGodleyTitle(id);},
    }));
    
    var popup=new GodleyTableWindow(namedItem.popup.getPrefix());
    
    switch (popup.clickTypeZoomed(x,y))
    {
      case "background": break;
      case "row0":
      menu.append(new MenuItem({
        label: "Add new stock variable",
        click: async ()=> {popup.addStockVar(x);},
      }));
      
      var importMenu=new Menu();
      var importables=popup.matchingTableColumns(x);
      for (var i in importables)
        importMenu.append(new MenuItem({
          label: importables[i],
          click: async (item)=> {popup.importStockVar(item.label,x);},
        }));
      
      menu.append(new MenuItem({
        label: "Import variable",
        submenu: importMenu,
      }));
      
      menu.append(new MenuItem({
        label: "Delete stock variable",
        click: async ()=> {popup.deleteStockVar(x);},
      }));
      break;
      case "col0":
      menu.append(new MenuItem({
        label: "Add flow",
        click: async ()=> {popup.addFlow(y);},
      }));
      menu.append(new MenuItem({
        label: "Delete flow",
        click: async ()=> {popup.deleteFlow(y);},
      }));
      break;
      case "internal": break;
    } // switch clickTypeZoomed
    
    var r=popup.rowYZoomed(y);
    var c=popup.colXZoomed(x);
    if (r!=popup.selectedRow() || c!=popup.selectedCol())
    {
      popup.selectedRow(r);
      popup.selectedCol(c);
      popup.insertIdx(0);
      popup.selectedIdx(0);
    }
    var cell=namedItem.table.getCell(r,c);
    if (cell.length>0 && (r!=1 || c!=0))
    {
      menu.append(new MenuItem({
        label: "Cut",
        click: async ()=> {popup.cut();}
      }));
      menu.append(new MenuItem({
        label: "Copy",
        click: async ()=> {popup.copy();}
      }));
    }
    
    if (r!=1 || c!=0)
    {
      var clip=minsky.clipboardEmpty();
      menu.append(new MenuItem({
        label: "Paste",
        enabled: !clip,
        click: async ()=> {popup.paste();}
      }));
    }
    menu.popup();
  }

}

