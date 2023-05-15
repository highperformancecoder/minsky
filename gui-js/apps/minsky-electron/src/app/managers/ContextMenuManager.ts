import {
  CanvasItem,
  ClassType,
  minsky, DataOp, GodleyIcon, Group, IntOp, Lock, OperationBase, PlotWidget, Ravel, Sheet, SwitchIcon,
  VariableBase, Functions, events
} from '@minsky/shared';
import { BrowserWindow, Menu, MenuItem, IpcMainEvent } from 'electron';
import { CommandsManager } from './CommandsManager';
import { WindowManager } from './WindowManager';
import * as log from 'electron-log';
import * as JSON5 from 'json5';

export class ContextMenuManager {
  private static x: number = null;
  private static y: number = null;

  private static showAllPlotsOnTabChecked = false;
  
  public static async initContextMenu(event: IpcMainEvent, x: number, y: number, type: string, command: string) {
    switch (type)
    {
      case "canvas":
      {
        const mainWindow = WindowManager.getMainWindow();
        this.x = x;
        this.y = y;
        
        const currentTab = WindowManager.currentTab;
        if (currentTab.equal(minsky.canvas))
          this.initContextMenuForWiring(mainWindow);
        else if (currentTab.equal(minsky.plotTab))
          this.initContextMenuForPlotTab(mainWindow);
      }
      break;
      case "godley":
      this.initContextMenuForGodleyPopup(command,x,y);
      break;
      case "html-godley":
      this.initContextMenuForGodleyHTMLPopup(event, command,x,y);
      break;
      case "html-summary":
      this.initContextMenuForSummaryTab(command);
      break;
      case "ravel":
      this.initContextMenuForRavelPopup(command,x,y);
      break;
      default:
      log.warn("Unknown context menu for ",type);
      break;
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
      const isWirePresent = minsky.canvas.getWireAt(this.x, this.y);

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
    const selectedItems = minsky.canvas.selection.items;
    const selectionSize = selectedItems.size();
    const ravelsSelected = minsky.canvas.ravelsSelected();

    const menuItems = [
      new MenuItem({
        label: 'Cut',
        click: () => {minsky.cut();},
        enabled: selectionSize > 0
      }),
      new MenuItem({
        label: 'Copy selection',
        click: () => {minsky.copy();},
        enabled: selectionSize > 0

      }),
      new MenuItem({
        label: 'Save selection as',
        click: async () => {
          await CommandsManager.saveSelectionAsFile();
        },
        enabled: selectionSize > 0
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
        click: () => {minsky.canvas.showPlotsOnTab();},
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
        label: 'Link selected Ravels',
        click: async () => {minsky.canvas.lockRavelsInSelection();},
        enabled: ravelsSelected > 1
      }),
      new MenuItem({
        label: 'Unlink selected Ravels',
        click: async () => {minsky.canvas.unlockRavelsInSelection();},
        enabled: ravelsSelected > 0
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
          WindowManager.currentTab.equal(minsky.canvas),
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
          ...(await ContextMenuManager.buildContextMenuForRavel(new Ravel(minsky.canvas.item))),
        ];

        break;

      case ClassType.Lock:
        menuItems = [
          ...menuItems,
          ...(await ContextMenuManager.buildContextMenuForLock()),
        ];

        break;

    case ClassType.Sheet:
      let sheet=new Sheet(minsky.canvas.item);
      menuItems = [
          ...menuItems,
        new MenuItem({
          label: 'Row Slices',
          submenu: [
            {
              label: 'Head',
              click: () => {sheet.showSlice("head");}
            },
            {
              label: 'Tail',
              click: () => {sheet.showSlice("tail");}
            },
              {
              label: 'Head & Tail',
              click: () => {sheet.showSlice("headAndTail");}
            },
         ]
        }),
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
    const plot=new PlotWidget(minsky.canvas.item);
    const displayPlotOnTabChecked = plot.plotTabDisplay();

    const menuItems = [
      new MenuItem({
        label: 'Expand',
        click: async () => {
          await CommandsManager.expandPlot(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Make Group Plot',
        click: async () => {plot.makeDisplayPlot();}
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
        click: async () => {plot.togglePlotTabDisplay();}
      }),
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV(plot);
        },
      }),
      new MenuItem({
        label: 'Export Image As',
        submenu: [
          {
            label: 'SVG',
            click: async () => {
              CommandsManager.exportItemAsImage(plot, 'svg', 'SVG');
            },
          },
          {
            label: 'PDF',
            click: async () => {
              CommandsManager.exportItemAsImage(plot, 'pdf', 'PDF');
            },
          },
          {
            label: 'PostScript',
            click: async () => {
              CommandsManager.exportItemAsImage(plot, 'ps', 'PostScript');
            },
          },
          {
            label: 'EMF',
            visible: Functions.isWindows(),
            click: async () => {
              CommandsManager.exportItemAsImage(plot, 'emf', 'EMF');
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
    let godley=new GodleyIcon(minsky.canvas.item);
    const displayVariableChecked = godley.variableDisplay();
    const rowColButtonsChecked = godley.buttonDisplay();
    const editorModeChecked = godley.editorMode();

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
        click: async () => {godley.toggleEditorMode();}
      }),
      new MenuItem({
        label: 'Row/Col buttons',
        checked: rowColButtonsChecked,
        type: 'checkbox',
        click: async () => {godley.toggleButtons();}
      }),
      new MenuItem({
        label: 'Display variables',
        type: 'checkbox',
        checked: displayVariableChecked,
        click: async () => {godley.toggleVariableDisplay();}
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
    let op=new OperationBase(minsky.canvas.item);

    try {
      var portValues = op.portValues();
    } catch (error) {
      var portValues = 'unknown';
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
    if (op.type() === 'data') {
      menuItems.push(
        new MenuItem({
          label: 'Import Data',
          click: async () => {
            const filePath = await CommandsManager.getFilePathUsingSaveDialog();
            if (filePath) {(new DataOp(op)).readData(filePath);}
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

    if (op.type() === 'integrate') {
      menuItems.push(
        new MenuItem({
          label: 'Toggle var binding',
          click: async () => {
            (new IntOp(op)).toggleCoupled();
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
    let group=new Group(minsky.canvas.item);
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
          group.removeDisplayPlot();
          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Copy',
        click: () => {minsky.canvas.copyItem();}
      }),
      new MenuItem({
        label: 'Make subroutine',
        click: () => {group.makeSubroutine();}
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
          group.flipContents();
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

  private static async buildContextMenuForRavel(ravel: Ravel): Promise<MenuItem[]> {
    const aggregations = [{label: 'Σ', value: 'sum'},{label: 'Π', value: 'prod'},{label:'av',value:'av'},{label: 'σ', value: 'stddev'},{label: 'min', value: 'min'},{label: 'max', value: 'max'}];

    const handleIndex = ravel.selectedHandle();
    const sortOrder = ravel.sortOrder();
    const editorMode = ravel.editorMode();

    const ravelsSelected = minsky.canvas.ravelsSelected();
    const allLockHandles = ravel.lockGroup.allLockHandles();

    const lockHandlesAvailable = ravelsSelected > 1 || Object.keys(allLockHandles).length !== 0;
    const unlockAvailable = Object.keys(allLockHandles).length !== 0;
    const handleAvailable = handleIndex !== -1;

    let menuItems = [
      new MenuItem({
        label: 'Editor mode',
        type: 'checkbox',
        checked: editorMode,
        click: () => {ravel.toggleEditorMode();}
      }),
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV(ravel);
        },
      }),
      new MenuItem({
        label: 'Set next aggregation',
        submenu: aggregations.map(agg => ({
          label: agg.label,
          click: async () => {ravel.nextReduction(agg.value);}
        }))
      }),
      new MenuItem({ 
        label: 'Link specific handles',
        click: async () => {
          CommandsManager.lockSpecificHandles(ravel);
        },
        enabled: lockHandlesAvailable 
      }),
      new MenuItem({
        label: 'Unlink',
        enabled: unlockAvailable,
        click: async () => {
          ravel.leaveLockGroup();
          CommandsManager.requestRedraw();
        },
      })
    ];

    let valueSort=(type: string,dir: string)=>{
      return type+dir[0].toUpperCase()+dir.slice(1);
    };
    menuItems.push(...[new MenuItem(
      {
        label: 'Toggle axis calipers',
        enabled: handleAvailable,
        click: async () => {
          ravel.toggleDisplayFilterCaliper();
          ravel.broadcastStateToLockGroup();
        }
      }),
      new MenuItem(
      {
        label: 'Sort axis',
        enabled: handleAvailable,
        submenu: ['none','forward','reverse'].map(so =>(<any>{
          label: so,
          type: 'radio',
          checked: sortOrder == so,
          click: () => {
            ravel.setSortOrder(valueSort('static',so));
            ravel.broadcastStateToLockGroup();
            minsky.reset();
          }
        })).concat(
          ['forward','reverse'].map(vso =>(<any>{
            label: `${vso} dynamically`,
            type: 'radio',
            checked: sortOrder == valueSort('dynamic',vso),
            click: async () => {
              ravel.sortByValue(vso);
              ravel.setSortOrder(valueSort('dynamic',vso));
              ravel.broadcastStateToLockGroup();
              minsky.reset();
          }
        })))
      }),
      new MenuItem({
        label: 'Pick axis slices',
        enabled: handleAvailable,
        click: async () => {
          await CommandsManager.pickSlices(ravel,handleIndex);
        }
      }),
    new MenuItem({
      label: 'Axis properties',
      enabled: handleAvailable,
      submenu: [
        {
          label: 'Description',
          click: async () => {
            await CommandsManager.editHandleDescription(ravel,handleIndex);
          }
        },
        {
          label: 'Dimension',
          click: async () => {
            await CommandsManager.editHandleDimension(ravel,handleIndex);
          }
        },
        {
          label: 'Set aggregation',
          submenu: aggregations.map(agg => ({
            label: agg.label,
            click: () => {
              ravel.handleSetReduction(handleIndex, agg.value);
            }
          }))
        }
      ]
    })]);

    return menuItems;
  }

  private static async buildContextMenuForSwitchIcon(): Promise<MenuItem[]> {
    let switchIcon=new SwitchIcon(minsky.canvas.item);
    const menuItems = [
      new MenuItem({
        label: 'Add case',
        click: () => {switchIcon.setNumCases(switchIcon.numCases()+1);},
      }),
      new MenuItem({
        label: 'Delete case',
        click: () => {switchIcon.setNumCases(switchIcon.numCases()-1);},
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
    let ravel=new Lock(minsky.canvas.item);
    return [
      new MenuItem({
        label: ravel.locked()? 'Unlock': 'Lock',
        click: async () => {ravel.toggleLocked();}
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

    let v=new VariableBase(minsky.canvas.item);
    
    const menuItems = [
      dims && dims.length
        ? new MenuItem({ label: `Dims ${dims.toString()}` })
        : new MenuItem({ label: `Value ${v.value()}` }),
      new MenuItem({
        label: "Local",
        type: 'checkbox',
        checked: v.local(),
        click: async () => {v.toggleLocal();}
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

    if (v.defined()) {
      menuItems.push(
        new MenuItem({
          label: 'Hide variable definition',
          click: () => {v.toggleVarTabDisplay();}
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

    if (v.type() === 'parameter') {
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
          await CommandsManager.exportItemAsCSV(v);
        },
      })
    );

    return menuItems;
  }

  private static async initContextMenuForGodleyHTMLPopup(event: IpcMainEvent, command: string, r: number, c: number)
  {
    const parsed = JSON5.parse(command);
    const godley=new GodleyIcon(parsed[0]);

    const refreshFunction = () => event.sender.send(events.GODLEY_POPUP_REFRESH);
      
    this.initContextMenuForGodley(godley, r, c, parsed[1], refreshFunction);
  }

  private static async initContextMenuForGodleyPopup(namedItemSubCommand: string, x: number, y: number)
  {
    //  console.log(`initContextMenuForGodleyPopup ${namedItemSubCommand}, ${x},${y}\n`);
    const godley=new GodleyIcon(namedItemSubCommand);

    const r=godley.popup.rowYZoomed(y);
    const c=godley.popup.colXZoomed(x);

    const clickType = godley.popup.clickTypeZoomed(x, y);

    this.initContextMenuForGodley(godley, r, c, clickType, () => {});
  }

  private static async initContextMenuForSummaryTab(valueId: string)
  {
    var menu=new Menu();
    menu.append(new MenuItem({
      label: "Show variable on canvas",
      click: async ()=> {await minsky.showVariableDefinitionOnCanvas(valueId);},
    }));
    menu.popup();
  }
  
  private static async initContextMenuForGodley(godley: GodleyIcon, r: number, c: number, clickType: string, refreshFunction: () => void)
  {
    var menu=new Menu();

    menu.append(new MenuItem({
      label: "Help",
      click: async ()=> {await CommandsManager.loadHelpFile("GodleyTable");},
    }));
    
    menu.append(new MenuItem({
      label: "Title",
      click: async ()=> {
        await CommandsManager.editGodleyTitle(godley.id());
        refreshFunction();
      },
    }));

    switch (clickType)
    {
      case "background": break;
      case "row0":
      menu.append(new MenuItem({
        label: "Add new stock variable",
        click: async ()=> {
          godley.popup.addStockVarByCol(c);
          refreshFunction();
        },
      }));
      
      var importMenu=new Menu();
      var importables=godley.popup.matchingTableColumnsByCol(c);
      for (var i in importables)
        importMenu.append(new MenuItem({
          label: importables[i],
          click: async (item)=> {
            godley.popup.importStockVarByCol(item.label, c);
            refreshFunction();
          },
        }));
      
      menu.append(new MenuItem({
        label: "Import variable",
        submenu: importMenu,
      }));
      
      menu.append(new MenuItem({
        label: "Delete stock variable",
        click: async ()=> {
          godley.popup.deleteStockVarByCol(c);
          refreshFunction();
        },
      }));
      break;
      case "col0":
      menu.append(new MenuItem({
        label: "Add flow",
        click: async ()=> {
          godley.popup.addFlowByRow(r);
          refreshFunction();
        },
      }));
      menu.append(new MenuItem({
        label: "Delete flow",
        click: async ()=> {
          godley.popup.deleteFlowByRow(r);
          refreshFunction();
        },
      }));
      break;
      case "internal": break;
    } // switch clickType
    
    if (r!=godley.popup.selectedRow() || c!=godley.popup.selectedCol())
    {
      godley.popup.selectedRow(r);
      godley.popup.selectedCol(c);
      godley.popup.insertIdx(0);
      godley.popup.selectIdx(0);
    }
    var cell=godley.table.getCell(r,c);
    if (cell.length>0 && (r!=1 || c!=0))
    {
      menu.append(new MenuItem({
        label: "Cut",
        click: async ()=> {
          godley.popup.cut();
          refreshFunction();
        }
      }));
      menu.append(new MenuItem({
        label: "Copy",
        click: async ()=> {godley.popup.copy();}
      }));
    }
    
    if (r!=1 || c!=0)
    {
      var clip=minsky.clipboardEmpty();
      menu.append(new MenuItem({
        label: "Paste",
        enabled: !clip,
        click: async ()=> {
          godley.popup.paste();
          refreshFunction();
        }
      }));
    }
    menu.popup();
  }
  
  private static async initContextMenuForRavelPopup(namedItemSubCommand: string, x: number, y: number)
  {
    let ravel=new Ravel(namedItemSubCommand);
    let menu=Menu.buildFromTemplate(await this.buildContextMenuForRavel(ravel));
    menu.popup();
  }
}

