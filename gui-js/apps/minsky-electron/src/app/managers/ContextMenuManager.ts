import {
  CanvasItem,ClassType,
  minsky, DataOp, GodleyIcon, Group, IntOp, Lock, OperationBase, PlotWidget, PubTab,
  Ravel, RenderNativeWindow, Sheet, SwitchIcon,VariableBase, VariableValue, Functions, events
} from '@minsky/shared';
import { BrowserWindow, Menu, MenuItem, IpcMainEvent } from 'electron';
import { BookmarkManager } from './BookmarkManager';
import { CommandsManager } from './CommandsManager';
import { WindowManager } from './WindowManager';
import log from 'electron-log';
import JSON5 from 'json5';

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
        if (currentTab.$equal(minsky.canvas))
          this.initContextMenuForWiring(mainWindow);
        else if (currentTab.$equal(minsky.phillipsDiagram))
          this.initContextMenuForPhillipsDiagram(mainWindow);
        else
          this.initContextMenuForPublication(event, mainWindow,currentTab);
      }
      break;
      case 'publication-button':
      this.initContextMenuForPublicationTabButton(event,command);
      break;
      case "godley":
      this.initContextMenuForGodleyPopup(command,x,y);
      break;
      case "html-godley":
      this.initContextMenuForGodleyHTMLPopup(event, command,x,y);
      break;
      case "ravel":
      this.initContextMenuForRavelPopup(command,x,y);
      break;
      case "csv-import":
      this.initContextMenuForCSVImport(event, command,x,y);
      break;
      default:
      log.warn("Unknown context menu for ",type);
      break;
    }
  }

  private static async initContextMenuForPublication(event: IpcMainEvent, mainWindow: BrowserWindow, currentTab: RenderNativeWindow) {
    let pubTab=new PubTab(currentTab);
    const menuItems = [
      new MenuItem({
        label: 'Add note: please just type note directly for now',
        click: ()=>{
        },
        enabled: false,
      }),
      new MenuItem({
        label: 'Rotate item',
        click: () => {
          pubTab.rotateItemAt(this.x,this.y);
        },
        enabled: await pubTab.getItemAt(this.x,this.y),
      }),
      new MenuItem({
        label: 'Toggle editor mode',
        click: () => {
          pubTab.toggleEditorModeAt(this.x,this.y);
        },
        enabled: await pubTab.getItemAt(this.x,this.y),
      }),
      new MenuItem({
        label: 'Remove item',
        click: () => {
          pubTab.removeItemAt(this.x,this.y);
        },
        enabled: await pubTab.getItemAt(this.x,this.y),
      }),
    ];

      ContextMenuManager.buildAndDisplayContextMenu(menuItems, mainWindow);

      return;
  }
  
  private static async initContextMenuForPublicationTabButton(event: IpcMainEvent, command: string) {
    let pubTab=new PubTab(command);
    const menuItems = [
      new MenuItem({
        label: 'Rename publication tab',
        click:  async () => {
          await CommandsManager.renamePubTab(command);
          event.sender.send(events.PUB_TAB_REMOVED);
        }
      }),
       new MenuItem({
        label: 'Remove publication tab',
        click: async () => {
          event.sender.send(events.CHANGE_MAIN_TAB);
          setTimeout(async ()=>{
            await pubTab.removeSelf();
            event.sender.send(events.PUB_TAB_REMOVED);
          },100); // allow enough time for renderer to switch to canvas tabs before removing this tab
        }
      }),
    ];

    ContextMenuManager.buildAndDisplayContextMenu(menuItems, WindowManager.getMainWindow());

    return;
  }
  
  private static async initContextMenuForPhillipsDiagram(mainWindow: BrowserWindow) {
      const menuItems = [
        new MenuItem({
          label: 'Rotate',
          click: async () => {
            minsky.phillipsDiagram.startRotatingItem(this.x,this.y);
          },
        }),
      ];

      ContextMenuManager.buildAndDisplayContextMenu(menuItems, mainWindow);
  }

  private static async initContextMenuForWiring(mainWindow: BrowserWindow) {
    try {
      const isWirePresent = await minsky.canvas.getWireAt(this.x, this.y);

      const isWireVisible = await minsky.canvas.wire.visible();
      const itemInfo = await CommandsManager.getItemInfo(this.x, this.y);

      if (isWirePresent && isWireVisible && (itemInfo?.classType != ClassType.Group || itemInfo?.displayContents)) {
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
        await ContextMenuManager.canvasContext(),
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
            await CommandsManager.editVar();
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

  private static async canvasContext(): Promise<MenuItem[]> {
    const selectedItems = minsky.canvas.selection.items;
    const selectionSize = await selectedItems.size();
    const ravelsSelected = await minsky.canvas.ravelsSelected();

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
        click: () => {
          minsky.openModelInCanvas();
          BookmarkManager.updateBookmarkList();
        }
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
          WindowManager.currentTab.$equal(minsky.canvas),
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
              click: () => {sheet.showRowSlice("head");}
            },
            {
              label: 'Tail',
              click: () => {sheet.showRowSlice("tail");}
            },
            {
              label: 'Head & Tail',
              click: () => {sheet.showRowSlice("headAndTail");}
            },
         ]
        }),
        new MenuItem({
          label: 'Column Slices',
          submenu: [
            {
              label: 'Head',
              click: () => {sheet.showColSlice("head");}
            },
            {
              label: 'Tail',
              click: () => {sheet.showColSlice("tail");}
            },
            {
              label: 'Head & Tail',
              click: () => {sheet.showColSlice("headAndTail");}
            },
         ]
        }),
        new MenuItem({
          label: 'Export as CSV',
          click: () => {CommandsManager.exportItemAsCSV(sheet);}
        }),
      ];
      break;
      default:
        break;
    }

    menuItems = [
      ...menuItems,
      new MenuItem({
        label: `Rotate ${itemInfo.classType}`,
        click:  () => {
          minsky.canvas.rotateItem(this.x, this.y);
        }
      }),
      new MenuItem({
        label: 'Add item to a publication tab',
        submenu: await ContextMenuManager.pubTabMenu(),
      }),
      new MenuItem({
        label: `Delete ${itemInfo.classType}`,
        click: () => {
          CommandsManager.deleteCurrentItemHavingId(itemInfo.id);
        },
      }),
    ];
    
    return menuItems;
  }

  private static async pubTabMenu() {
    let pubTabs=await minsky.publicationTabs.$properties();
    let menu=[];
    for (let i=0; i<pubTabs.length; ++i) {
      let j=i;
      menu.push({
        label: pubTabs[i].name,
        click: ()=>{minsky.addCanvasItemToPublicationTab(j);}
      });
    }
    return menu;
  }
  
  private static async buildContextMenuForPlotWidget(
    itemInfo: CanvasItem
  ): Promise<MenuItem[]> {
    const plot=new PlotWidget(minsky.canvas.item);

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
        label: 'Set options as default',
        click: async () => {
          minsky.canvas.setDefaultPlotOptions();
        },
      }),
      new MenuItem({
        label: 'Apply default options',
        click: async () => {
          minsky.canvas.applyDefaultPlotOptions();
        },
      }),
      new MenuItem({
        label: 'Pen Styles',
        click: () => {
          WindowManager.createPopupWindowWithRouting({
            title: `Pen Styles`,
            url: `#/headless/menu/context-menu/pen-styles`,
            height: 700,
            width: 350,
          });
        },
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
              CommandsManager.exportItemAsImageDialog(plot, 'svg', 'SVG');
            },
          },
          {
            label: 'PDF',
            click: async () => {
              CommandsManager.exportItemAsImageDialog(plot, 'pdf', 'PDF');
            },
          },
          {
            label: 'PostScript',
            click: async () => {
              CommandsManager.exportItemAsImageDialog(plot, 'ps', 'PostScript');
            },
          },
          {
            label: 'EMF',
            visible: Functions.isWindows(),
            click: async () => {
              CommandsManager.exportItemAsImageDialog(plot, 'emf', 'EMF');
            },
          },
          {
            label: 'Portale Network Graphics',
            click: async () => {
              CommandsManager.exportItemAsImageDialog(plot, 'png', 'Portale Network Graphics');
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
    const displayVariableChecked = await godley.variableDisplay();
    const rowColButtonsChecked = await godley.buttonDisplay();
    const editorModeChecked = await godley.editorMode();

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
          CommandsManager.editGodleyTitle(godley);
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

    let portValues;
    await op.portValues().
      then((x)=>{portValues=x;}).
      catch((x)=>{portValues = 'unknown';});

    let menuItems = [
      new MenuItem({ label: `Port values ${portValues}` }),
      new MenuItem({
        label: 'Edit',
        click: async () => {
          await CommandsManager.editItem(itemInfo.classType);
        },
      }),
    ];

    // TODO data is obsolete
    if (await op.type() === 'data') {
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

    if (await op.type() === 'integrate') {
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
        click: async () => {
          minsky.openGroupInCanvas();
          BookmarkManager.updateBookmarkList();
        }
      }),
      new MenuItem({
        label: 'Zoom to display',
        click: async () => {
          minsky.canvas.zoomToDisplay();
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
        label: 'Resize icon on contents',
        click: () => {
          group.resizeOnContents();
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

    const handleIndex = await ravel.selectedHandle();
    const sortOrder = await ravel.sortOrder();
    const editorMode = await ravel.editorMode();

    const ravelsSelected = await minsky.canvas.ravelsSelected();
    // may be null if ravel.lockGroup is null, ie ravel is not part of a "RavelLockGroup"
    const allLockHandles = await ravel.lockGroup.allLockHandles();

    const linkHandlesAvailable = ravelsSelected > 1 || allLockHandles!==null && allLockHandles.length>0;
    const unlinkAvailable = allLockHandles!==null;
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
        label: 'Collapse all handles',
        click: async () => {ravel.collapseAllHandles(true);}
      }),
      new MenuItem({
        label: 'Expand all handles',
        click: async () => {ravel.collapseAllHandles(false);}
      }),
      new MenuItem({
        label: 'Flip',
        click: async () => {ravel.flipped(!await ravel.flipped());}
      }),
      new MenuItem({ 
        label: 'Link specific handles',
        click: async () => {
          CommandsManager.lockSpecificHandles(ravel);
        },
        enabled: linkHandlesAvailable 
      }),
      new MenuItem({
        label: 'Unlink',
        enabled: unlinkAvailable,
        click: async () => {
          ravel.leaveLockGroup();
          CommandsManager.requestRedraw();
        },
      })
    ];

    let valueSort=(type: string,dir: string)=>{
      return type+dir[0].toUpperCase()+dir.slice(1);
    };
    menuItems.push(...[
      new MenuItem({
        label: 'Toggle axis calipers',
        enabled: handleAvailable,
        click: async () => {
          ravel.toggleDisplayFilterCaliper();
          ravel.broadcastStateToLockGroup();
        }
      }),
      new MenuItem({
        label: 'Sort axis',
        enabled: handleAvailable,
        submenu: ['none','forward','reverse'].map(so =>(<any>{
          label: so,
          type: 'radio',
          checked: sortOrder == so,
          click: () => {
            ravel.setSortOrder(so);
            ravel.broadcastStateToLockGroup();
            minsky.requestReset();
          }
        })),
      }),
      new MenuItem({
        label: 'Sort by value',
        enabled: await ravel.handleSortableByValue(),
        submenu: [
          <any>{
            label: 'none',
            type: 'radio',
            checked: sortOrder == 'none',
            click: () => {
              ravel.setSortOrder('none');
              ravel.broadcastStateToLockGroup();
              minsky.requestReset();
            },
          }].concat(
            ['forward','reverse'].map(vso =>(<any>{
              label: `${vso} statically`,
              type: 'radio',
              checked: sortOrder == valueSort('static',vso),
              click: async () => {
                ravel.sortByValue(vso);
                ravel.setSortOrder(valueSort('static',vso));
                ravel.broadcastStateToLockGroup();
                minsky.reset();
              }
            }))).concat(
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
              }))),
      }),
      new MenuItem({
        label: 'Pick axis slices',
        enabled: handleAvailable,
        click: () => {
          CommandsManager.pickSlices(ravel,handleIndex);
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
        click: async () => {switchIcon.setNumCases(await switchIcon.numCases()+1);},
      }),
      new MenuItem({
        label: 'Delete case',
        click: async () => {switchIcon.setNumCases(await switchIcon.numCases()-1);},
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
        label: (await ravel.locked())? 'Unlock': 'Lock',
        click: () => {ravel.toggleLocked();}
      }),
      new MenuItem({
        label: 'Apply state to Ravel',
        click: () => {ravel.applyLockedStateToRavel();}
      }),
      
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
    let miniPlot=await v.miniPlotEnabled();
    
    const menuItems = [
      dims && dims.length
        ? new MenuItem({ label: `Dims ${await dims.toString()}` })
        : new MenuItem({ label: `Value ${await v.value()}` }),
      new MenuItem({
        label: "Local",
        type: 'checkbox',
        checked: await v.local(),
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
      new MenuItem({
        label: 'Mini Plot',
        type: 'checkbox',
        checked: miniPlot,
        click: () => {v.miniPlotEnabled(!miniPlot);}
      }),
    ];

    menuItems.push(
      new MenuItem({
        label: 'Flip',
        click: async () => {
          await CommandsManager.flip();
        },
      })
    );

    if (await v.type() === 'parameter') {
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
    const godley=new GodleyIcon(namedItemSubCommand);
    const r=await godley.popup.rowYZoomed(y);
    const c=await godley.popup.colXZoomed(x);
    const clickType = await godley.popup.clickTypeZoomed(x, y);
    this.initContextMenuForGodley(godley, r, c, clickType, () => {});
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
        await CommandsManager.editGodleyTitle(godley);
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
      var importables=await godley.popup.matchingTableColumnsByCol(c);
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
    
    if (r!=await godley.popup.selectedRow() || c!=await godley.popup.selectedCol())
    {
      godley.popup.selectedRow(r);
      godley.popup.selectedCol(c);
      godley.popup.insertIdx(0);
      godley.popup.selectIdx(0);
    }
    var cell=await godley.table.getCell(r,c);
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
      var clip=await minsky.clipboardEmpty();
      menu.append(new MenuItem({
        label: "Paste",
        enabled: !clip,
        click: ()=> {
          godley.popup.paste();
          refreshFunction();
        }
      }));
    }
    const flows=await godley.table.getVariables();
    var flowMenu=new Menu();
    for (let i=0; i<flows.length; ++i)
      flowMenu.append(new MenuItem({
        label: flows[i],
        submenu: [
          {
            label: '+',
            click: ()=>{
              godley.table.setCell(r,c,flows[i]);
              refreshFunction();
            }
          },
          {
            label: '-',
            click: ()=>{
              godley.table.setCell(r,c,`-${flows[i]}`);
              refreshFunction();
            }
          },
        ]            
      }));
    menu.append(new MenuItem({
      label: 'Insert flow',
      submenu: flowMenu,
    }));

    if ((await godley.table._assetClass())[c]==="equity")
      menu.append(new MenuItem({
        label: 'Balance equity',
        click: ()=>{
          godley.table.balanceEquity(c);
          refreshFunction();
        }
      }));
    
    menu.popup();
  }
  
  private static async initContextMenuForRavelPopup(namedItemSubCommand: string, x: number, y: number)
  {
    let ravel=new Ravel(namedItemSubCommand);
    let menu=Menu.buildFromTemplate(await this.buildContextMenuForRavel(ravel));
    menu.popup();
  }

  private static async initContextMenuForCSVImport(event: IpcMainEvent, variableValue: string, row: number, col: number)
  {
    const refresh=()=>event.sender.send(events.CSV_IMPORT_REFRESH);
    const value=new VariableValue(variableValue);
    var menu=Menu.buildFromTemplate([
      new MenuItem({
        label: 'Set as header row',
        click: ()=>{
          value.csvDialog.spec.headerRow(row);
          refresh();
        },
      }),
      new MenuItem({
        label: 'Auto-classify columns as axis/data',
        click: async ()=>{
          value.csvDialog.classifyColumns();
          refresh();
        },
      }),
      new MenuItem({
        label: 'Populate column labels',
        click: async ()=>{
          value.csvDialog.populateHeaders();
          refresh();
        },
      }),
      new MenuItem({
        label: 'Populate current column label',
        click: ()=>{
          value.csvDialog.populateHeader(col);
          refresh();
        },
      }),
      new MenuItem({
        label: 'Set start of data row, and column',
        click: ()=>{
          value.csvDialog.spec.setDataArea(row,col);
          refresh();
        },
      }),
      new MenuItem({
        label: 'Set start of data row',
        click: async ()=>{
          let c=await value.csvDialog.spec.nColAxes();
          value.csvDialog.spec.setDataArea(row,c);
          refresh();
        },
      }),
      new MenuItem({
        label: 'Set start of data column',
        click: async ()=>{
          let r=await value.csvDialog.spec.nRowAxes();
          value.csvDialog.spec.setDataArea(r,col);
          refresh();
        },
      }),
      ]);
    menu.popup();
  }
}

