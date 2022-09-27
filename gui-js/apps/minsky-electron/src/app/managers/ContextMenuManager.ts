import {
  CanvasItem,
  ClassType,
  commandsMapping,
  isEmptyObject,
  isWindows,
  MainRenderingTabs,
} from '@minsky/shared';
import { BrowserWindow, Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { RestServiceManager } from './RestServiceManager';
import { WindowManager } from './WindowManager';
import { GodleyPopup } from './GodleyMenuManager';
import * as log from 'electron-log';
import * as JSON5 from 'json5';

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
    const rowNumber = await RestServiceManager.handleMinskyProcess({
      command: `${commandsMapping.VARIABLE_TAB_ROW_Y} ${this.y}`,
    });

    const clickType = await RestServiceManager.handleMinskyProcess({
      command: `${commandsMapping.VARIABLE_TAB_CLICK_TYPE} [${this.x}, ${this.y}]`,
    });

    if (clickType === `internal`) {
      const varName = await RestServiceManager.handleMinskyProcess({
        command: `${commandsMapping.VARIABLE_TAB_GET_VAR_NAME} ${rowNumber}`,
      });

      const menuItems = [
        new MenuItem({
          label: `Remove ${varName} from tab`,
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: `${commandsMapping.VARIABLE_TAB_TOGGLE_VAR_DISPLAY} ${rowNumber}`,
            });

            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.REQUEST_REDRAW_SUBCOMMAND,
            });
          },
        }),
      ];

      ContextMenuManager.buildAndDisplayContextMenu(menuItems, mainWindow);

      return;
    }
  }

  private static async initContextMenuForPlotTab(mainWindow: BrowserWindow) {
    const isPlot = await RestServiceManager.handleMinskyProcess({
      command: `${commandsMapping.PLOT_TAB_GET_ITEM_AT} [${this.x},${this.y}]`,
    });

    if (isPlot) {
      const menuItems = [
        new MenuItem({
          label: `Remove plot from tab`,
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.PLOT_TAB_TOGGLE_PLOT_DISPLAY,
            });

            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.REQUEST_REDRAW_SUBCOMMAND,
            });
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

      const isWireVisibleRes = await RestServiceManager.handleMinskyProcess({
        command: commandsMapping.CANVAS_WIRE_VISIBLE,
      });

      const isWireVisible = !isEmptyObject(isWireVisibleRes);
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
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_COPY_ITEM,
            });
          },
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
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_COPY_ITEM,
            });
          },
        }),
        new MenuItem({
          label: 'Remove',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_REMOVE_ITEM_FROM_ITS_GROUP,
            });
          },
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
        click: () => {
          CommandsManager.postNote('wire');
        },
      }),
      new MenuItem({
        label: 'Straighten',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_WIRE_STRAIGHTEN,
          });
        },
      }),
      new MenuItem({
        label: 'Delete wire',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_DELETE_WIRE,
          });
        },
      }),
    ];

    return menuItems;
  }

  private static canvasContext(): MenuItem[] {
    const menuItems = [
      new MenuItem({
        label: 'Cut',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CUT,
          });
        },
      }),
      new MenuItem({
        label: 'Copy selection',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.COPY,
          });
        },
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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_PUSH_DEFINING_VARS_TO_TAB,
          });

          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Show all defining groups on canvas',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_SHOW_DEFINING_VARS_ON_CANVAS,
          });

          await CommandsManager.requestRedraw();
        },
      }),

      new MenuItem({
        label: 'Show all plots on tab',
        type: 'checkbox',
        checked: this.showAllPlotsOnTabChecked,
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_SHOW_ALL_PLOTS_ON_TAB,
          });

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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.GROUP_SELECTION,
          });
        },
      }),
      new MenuItem({
        label: 'Lock selected Ravels',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_LOCK_RAVELS_IN_SELECTION,
          });
        },
      }),
      new MenuItem({
        label: 'Unlock selected Ravels',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_UNLOCK_RAVELS_IN_SELECTION,
          });
        },
      }),
      new MenuItem({
        label: 'Open master group',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_OPEN_MODEL_IN_CANVAS,
          });
        },
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
            click: async () => {
              await RestServiceManager.handleMinskyProcess({
                command: commandsMapping.CANVAS_COPY_ITEM,
              });
            },
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
    const displayPlotOnTabChecked = (await RestServiceManager.handleMinskyProcess(
      {
        command: commandsMapping.CANVAS_ITEM_GET_PLOT_TAB_DISPLAY,
      }
    )) as boolean;

    const menuItems = [
      new MenuItem({
        label: 'Expand',
        click: async () => {
          await CommandsManager.expandPlot(itemInfo);
        },
      }),
      new MenuItem({
        label: 'Make Group Plot',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_MAKE_DISPLAY_PLOT,
          });
        },
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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_TOGGLE_PLOT_TAB_DISPLAY,
          });
        },
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
    const displayVariableChecked = (await RestServiceManager.handleMinskyProcess(
      {
        command: commandsMapping.CANVAS_ITEM_GET_VARIABLE_DISPLAY,
      }
    )) as boolean;

    const rowColButtonsChecked = (await RestServiceManager.handleMinskyProcess({
      command: commandsMapping.CANVAS_ITEM_GET_BUTTON_DISPLAY,
    })) as boolean;

    const editorModeChecked = (await RestServiceManager.handleMinskyProcess({
      command: commandsMapping.CANVAS_ITEM_GET_EDITOR_MODE,
    })) as boolean;

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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_TOGGLE_EDITOR_MODE,
          });
        },
      }),
      new MenuItem({
        label: 'Row/Col buttons',
        checked: rowColButtonsChecked,
        type: 'checkbox',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_TOGGLE_BUTTONS,
          });
        },
      }),
      new MenuItem({
        label: 'Display variables',
        type: 'checkbox',
        checked: displayVariableChecked,
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_TOGGLE_VARIABLE_DISPLAY,
          });
        },
      }),
      new MenuItem({
        label: 'Copy flow variables',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_COPY_ALL_FLOW_VARS,
          });
        },
      }),
      new MenuItem({
        label: 'Copy stock variables',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_COPY_ALL_STOCK_VARS,
          });
        },
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
      portValues = (await RestServiceManager.handleMinskyProcess({
        command: commandsMapping.CANVAS_ITEM_PORT_VALUES,
      })) as string;
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

    if ((await CommandsManager.getItemType()) === 'data') {
      menuItems.push(
        new MenuItem({
          label: 'Import Data',
          click: async () => {
            const filePath = await CommandsManager.getFilePathUsingSaveDialog();

            if (filePath) {
              await RestServiceManager.handleMinskyProcess({
                command: `${commandsMapping.CANVAS_ITEM_READ_DATA} ${filePath}`,
              });
            }
          },
        })
      );
    }

    menuItems = [
      ...menuItems,
      new MenuItem({
        label: 'Copy item',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_COPY_ITEM,
          });
        },
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
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_ITEM_TOGGLE_COUPLED,
            });
            CommandsManager.requestRedraw();
          },
        })
      );
      menuItems.push(
        new MenuItem({
          label: 'Select all instances',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_SELECT_ALL_VARIABLES,
            });
          },
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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_OPEN_GROUP_IN_CANVAS,
          });
        },
      }),
      new MenuItem({
        label: 'Zoom to display',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ZOOM_TO_DISPLAY,
          });

          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Remove plot icon',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_REMOVE_DISPLAY_PLOT,
          });

          await CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Copy',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_COPY_ITEM,
          });
        },
      }),
      new MenuItem({
        label: 'Make subroutine',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: "/minsky/canvas/item/makeSubroutine",
          });
        },
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
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ITEM_FLIP_CONTENTS,
          });
          CommandsManager.requestRedraw();
        },
      }),
      new MenuItem({
        label: 'Ungroup',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_UNGROUP_ITEM,
          });
          CommandsManager.requestRedraw();
        },
      }),
    ];

    return menuItems;
  }

  private static async buildContextMenuForRavel(): Promise<MenuItem[]> {
    const aggregations = [{label: 'Σ', value: 'sum'},{label: 'Π', value: 'prod'},{label: 'σ', value: 'stddev'},{label: 'min', value: 'min'},{label: 'max', value: 'max'}];

    const handleIndex = <number>await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_RAVEL_HANDLE_SELECTED });
    const sortOrder = await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_RAVEL_HANDLE_GET_SORT_ORDER });
    const editorMode = (await RestServiceManager.handleMinskyProcess(
      {
        command: "/minsky/canvas/item/editorMode",
      }
    )) as boolean;

    let menuItems = [
      new MenuItem({
        label: 'Editor mode',
        type: 'checkbox',
        checked: editorMode,
        click: async () => {
          await RestServiceManager.handleMinskyProcess(
            {
              command: "/minsky/canvas/item/toggleEditorMode",
            }
          );
        },
      }),
      new MenuItem({
        label: 'Export as CSV',
        click: async () => {
          await CommandsManager.exportItemAsCSV();
        },
      }),
      new MenuItem({
        label: 'Set next aggregation',
        submenu: aggregations.map(agg => ({
          label: agg.label,
          click: async () => {
            await RestServiceManager.handleMinskyProcess({command: `${commandsMapping.CANVAS_RAVEL_NEXT_REDUCTION} "${agg.value}"` });
          }
        }))
      }),
      new MenuItem({
        label: 'Axis properties',
        submenu: [
          {
            label: 'Description',
            click: async () => {
              await CommandsManager.editHandleDescription(handleIndex);
            }
          },
          {
            label: 'Dimension',
            click: async () => {
              await CommandsManager.editHandleDimension(handleIndex);
            }
          },
          {
            label: 'Toggle calipers',
            click: async () => {
              await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_RAVEL_TOGGLE_CALIPERS });
              await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_RAVEL_BROADCAST_TO_LOCKGROUP });
              
            }
          },
          {
            label: 'Set aggregation',
            submenu: aggregations.map(agg => ({
              label: agg.label,
              click: async () => {
                const args = JSON5.stringify([handleIndex, agg.value]);
                await RestServiceManager.handleMinskyProcess({ command: `${commandsMapping.CANVAS_RAVEL_HANDLE_SET_REDUCTION} ${args}` });
              }
            }))
          },
          {
            label: 'Sort',
            submenu: ['none','forward','reverse'].map(so =>({
              label: so,
                type: 'radio',
                checked: sortOrder == so,
                click: async () => {
                  await RestServiceManager.handleMinskyProcess({ command: `${commandsMapping.CANVAS_RAVEL_HANDLE_SET_SORT_ORDER} ${JSON5.stringify(so)}` });
                  await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_RAVEL_BROADCAST_TO_LOCKGROUP });
                  await RestServiceManager.handleMinskyProcess({ command: commandsMapping.RESET });
                }
            })).concat(
              ['forward','reverse'].map(vso =>(<any>{
              label: `${vso} by value`,
              click: async () => {
                await RestServiceManager.handleMinskyProcess({ command: `${commandsMapping.CANVAS_RAVEL_HANDLE_SET_SORT_BY_VALUE} ${JSON5.stringify(vso)}` });
                await RestServiceManager.handleMinskyProcess({ command: commandsMapping.RESET });
              }
            })))
          },
          {
            label: 'Pick slices',
            click: async () => {
              await CommandsManager.pickSlices(handleIndex);
            }
          }
        ]
      }),
      new MenuItem({ 
        label: 'Lock specific handles',
        click: async () => {
          CommandsManager.lockSpecificHandles();
        } 
      }),
      new MenuItem({
        label: 'Unlock',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({ command: commandsMapping.CANVAS_ITEM_LEAVE_LOCK_GROUP });
          CommandsManager.requestRedraw();
        },
      })
    ];

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
    const menuItems = [];

    const isItemLocked = await CommandsManager.isItemLocked();

    const toggleLocked = async () => {
      await RestServiceManager.handleMinskyProcess({
        command: commandsMapping.CANVAS_ITEM_TOGGLE_LOCKED,
      });
    };

    if (isItemLocked) {
      menuItems.push(
        new MenuItem({
          label: 'Unlock',
          click: async () => {
            await toggleLocked();
          },
        })
      );
    } else {
      menuItems.push(
        new MenuItem({
          label: 'Lock',
          click: async () => {
            await toggleLocked();
          },
        })
      );
    }

    return menuItems;
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

    const local = (await RestServiceManager.handleMinskyProcess(
      {
        command:"/minsky/canvas/item/local" ,
      }
    )) as boolean;
    
    const menuItems = [
      dims && dims.length
        ? new MenuItem({ label: `Dims ${dims.toString()}` })
        : new MenuItem({ label: `Value ${itemInfo?.value || ''}` }),
      new MenuItem({
        label: "Local",
        type: 'checkbox',
        checked: local,
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: "/minsky/canvas/item/toggleLocal",
          });
        },
      }),
      new MenuItem({
        label: 'Find definition',
        click: async () => {
          await CommandsManager.findDefinition();
        },
      }),
      new MenuItem({
        label: 'Select all instances',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_SELECT_ALL_VARIABLES,
          });
        },
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
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_COPY_ITEM,
          });
        },
      }),
      new MenuItem({
        label: 'Add integral',
        click: async () => {
          await RestServiceManager.handleMinskyProcess({
            command: commandsMapping.CANVAS_ADD_INTEGRAL,
          });
        },
      }),
    ];

    if (await CommandsManager.isItemDefined()) {
      menuItems.push(
        new MenuItem({
          label: 'Display variable on tab',
          click: async () => {
            await RestServiceManager.handleMinskyProcess({
              command: commandsMapping.CANVAS_ITEM_TOGGLE_VAR_TAB_DISPLAY,
            });
          },
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
    var menu=new Menu();
    menu.append(new MenuItem({
      label: "Help",
      click: async ()=> {await CommandsManager.loadHelpFile("GodleyTable");},
    }));

    var id=await RestServiceManager.handleMinskyProcess({
      command: `${namedItemSubCommand}/id`,
    }) as number;
    menu.append(new MenuItem({
      label: "Title",
      click: async ()=> {await CommandsManager.editGodleyTitle(id);},
    }));

    var popup=new GodleyPopup(`${namedItemSubCommand}/popup`);
    
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
    var cell=await RestServiceManager.handleMinskyProcess({
      command: `${namedItemSubCommand}/table/getCell [${r},${c}]`,
    }) as string;
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
      var clip=await RestServiceManager.handleMinskyProcess({
      command: "/minsky/clipboardEmpty",
      }) as boolean;
      menu.append(new MenuItem({
        label: "Paste",
        enabled: !clip,
        click: async ()=> {popup.paste();}
      }));
    }
    menu.popup();
  }

}
