import {
  commandsMapping,
  events,
  importCSVerrorMessage,
  MainRenderingTabs,
  MinskyProcessPayload,
  normalizeFilePathForPlatform,
  USE_FRONTEND_DRIVEN_RENDERING,
} from '@minsky/shared';
import { dialog, ipcMain } from 'electron';
import * as log from 'electron-log';
import { join } from 'path';
import { Utility } from '../utility';
import { RecordingManager } from './RecordingManager';
import { MinskyPreferences, StoreManager } from './StoreManager';
import { WindowManager } from './WindowManager';
const JSON5 = require('json5');

const addonDir = Utility.isPackaged()
  ? '../../node-addons'
  : '../../../node-addons';

// eslint-disable-next-line @typescript-eslint/no-var-requires

let restService = null;
try {
  restService = require('bindings')(join(addonDir, 'minskyRESTService.node'));
} catch (error) {
  log.error(error);
}

interface QueueItem {
  promise: Deferred;
  payload: MinskyProcessPayload;
}

class Deferred {
  public promise;
  public reject;
  public resolve;

  constructor() {
    this.promise = new Promise((resolve, reject) => {
      this.reject = reject;
      this.resolve = resolve;
    });
  }
}

restService.setMessageCallback(function (msg: string, buttons: string[]) {
  if (msg)
    return dialog.showMessageBoxSync({
      message: msg,
      type: 'info',
      buttons: buttons,
    });
});

restService.setBusyCursorCallback(function (busy: boolean) {
  WindowManager.getMainWindow().webContents.insertCSS(
    busy ? 'html body {cursor: wait}' : 'html body {cursor: default}'
  );
});

// TODO refactor to use command and arguments separately
function callRESTApi(command: string) {
  const {
    leftOffset,
    canvasWidth,
    canvasHeight,
    electronTopOffset,
    scaleFactor,
  } = WindowManager;

  //console.log('In callRESTApi::', command, ' | Window Values: left offset = ', leftOffset, '| Canvas Dims =', canvasWidth, canvasHeight, '| ETO=', electronTopOffset, '| Scale Factor = ', scaleFactor);

  if (!command) {
    log.error('callRESTApi called without any command');
    return {};
  }
  if (!restService) {
    log.error('Rest Service not ready');
    return {};
  }
  const commandMetaData = command.split(' ');
  const [cmd] = commandMetaData;
  let arg = '';
  if (commandMetaData.length >= 2) {
    arg = command.substring(command.indexOf(' ') + 1);
  }
  try {
    log.info('Rest API Call: ' + cmd + ': ' + arg);
    const response = restService.call(cmd, arg);
    log.info('Rest API Response: ' + response);
    return JSON5.parse(response);
  } catch (error) {
    if (cmd === commandsMapping.CANVAS_ITEM_IMPORT_FROM_CSV) {
      return importCSVerrorMessage;
    } else {
      if (error?.message) dialog.showErrorBox(error.message, '');
      log.error('Exception caught: ' + error?.message);
      return error?.message;
    }
  }
}

export class RestServiceManager {
  static currentMinskyModelFilePath: string;

  private static lastMouseMovePayload: MinskyProcessPayload = null;
  private static lastModelMoveToPayload: MinskyProcessPayload = null;
  private static payloadDataQueue: Array<QueueItem> = [];
  private static runningCommand = false;
  private static isQueueEnabled = true;
  private static render = true;
  private static lastZoomPayload: MinskyProcessPayload = null;
  static availableOperationsMappings: Record<string, string[]> = {};

  private static currentTab: MainRenderingTabs = MainRenderingTabs.canvas;

  public static async setCurrentTab(tab: MainRenderingTabs) {
    if (tab !== this.currentTab) {
      // disable the old tab
      this.handleMinskyProcess({
        command: this.currentTab + '/enabled false',
      });
      this.currentTab = tab;
      this.render = true;
      this.lastMouseMovePayload = null;
      this.lastModelMoveToPayload = null;
      this.lastZoomPayload = null;
      this.handleMinskyProcess({
        command: commandsMapping.RENDER_FRAME_SUBCOMMAND,
      });
    }
  }

  public static getCurrentTab(): MainRenderingTabs {
    return this.currentTab;
  }

  public static async reInvokeRenderFrame() {
    await this.handleMinskyProcess({
      command: commandsMapping.RENDER_FRAME_SUBCOMMAND,
    });
  }

  private static async processCommandsInQueue(): Promise<unknown> {
    // Should be on a separate thread......? -Janak
    const shouldProcessQueue = this.isQueueEnabled
      ? !this.runningCommand && this.payloadDataQueue.length > 0
      : this.payloadDataQueue.length > 0;

    if (shouldProcessQueue) {
      const nextItem = this.payloadDataQueue.shift();

      if (nextItem.payload.command === commandsMapping.MOUSEMOVE_SUBCOMMAND) {
        this.lastMouseMovePayload = null;
      } else if (nextItem.payload.command === commandsMapping.MOVE_TO) {
        this.lastModelMoveToPayload = null;
      } else if (nextItem.payload.command === commandsMapping.ZOOM_IN) {
        this.lastZoomPayload = null;
      }
      this.runningCommand = true;
      const res = await this.handleMinskyPayload(nextItem.payload);
      nextItem.promise.resolve(res);
    }
    return;
  }

  private static async resumeQueueProcessing(): Promise<unknown> {
    this.runningCommand = false;
    return await this.processCommandsInQueue();
  }

  public static async handleMinskyProcess(
    payload: MinskyProcessPayload
  ): Promise<unknown> {
    const wasQueueEmpty = this.payloadDataQueue.length === 0;

    const shouldProcessQueue = this.isQueueEnabled
      ? !this.runningCommand && wasQueueEmpty
      : true;

    let queueItem: QueueItem = null;

    // TODO:: Take into account Tab when merging commands
    if (payload.command === commandsMapping.MOUSEMOVE_SUBCOMMAND) {
      if (this.lastMouseMovePayload !== null) {
        // console.log("Merging mouse move commands");
        this.lastMouseMovePayload.mouseX = payload.mouseX;
        this.lastMouseMovePayload.mouseY = payload.mouseY;
      } else {
        queueItem = { payload, promise: new Deferred() };
        this.lastMouseMovePayload = payload;
        this.payloadDataQueue.push(queueItem);
      }
      this.lastModelMoveToPayload = null;
      this.lastZoomPayload = null;
    } else if (payload.command === commandsMapping.MOVE_TO) {
      if (this.lastModelMoveToPayload !== null) {
        this.lastModelMoveToPayload.mouseX = payload.mouseX;
        this.lastModelMoveToPayload.mouseY = payload.mouseY;
      } else {
        queueItem = { payload, promise: new Deferred() };
        this.lastModelMoveToPayload = payload;
        this.payloadDataQueue.push(queueItem);
      }
      this.lastMouseMovePayload = null;
      this.lastZoomPayload = null;
    } else if (payload.command === commandsMapping.ZOOM_IN) {
      if (this.lastZoomPayload !== null) {
        this.lastZoomPayload.args.x = payload.args.x;
        this.lastZoomPayload.args.y = payload.args.y;
        (this.lastZoomPayload.args.zoomFactor as number) *= payload.args
          .zoomFactor as number;
      } else {
        queueItem = { payload, promise: new Deferred() };
        this.lastZoomPayload = payload;
        this.payloadDataQueue.push(queueItem);
      }
      this.lastMouseMovePayload = null;
      this.lastModelMoveToPayload = null;
    } else {
      this.lastMouseMovePayload = null;
      this.lastModelMoveToPayload = null;
      this.lastZoomPayload = null;
      queueItem = { payload, promise: new Deferred() };
      this.payloadDataQueue.push(queueItem);
    }
    if (shouldProcessQueue) {
      // Control will come here when a new command comes after the whole queue was processed
      await this.processCommandsInQueue();
    }

    if (queueItem) {
      return queueItem.promise.promise;
    }
    return null;
  }

  private static async handleMinskyPayload(
    payload: MinskyProcessPayload
  ): Promise<unknown> {
    let res = null;

    switch (payload.command) {
      case commandsMapping.RECORD:
        await RecordingManager.handleRecord();
        break;

      case commandsMapping.RECORDING_REPLAY:
        await RecordingManager.handleRecordingReplay();
        break;

      case commandsMapping.AVAILABLE_OPERATIONS_MAPPING:
        res = this.availableOperationsMappings;
        break;

      default:
        res = await this.executeCommandOnMinskyServer(payload);
        break;
    }
    await this.resumeQueueProcessing();
    return res;
  }

  private static async executeCommandOnMinskyServer(
    payload: MinskyProcessPayload
  ): Promise<unknown> {
    let stdinCommand = null;

    switch (payload.command) {
      case commandsMapping.LOAD:
        stdinCommand = `${payload.command} ${payload.filePath}`;
        this.render = true;
        break;

      case commandsMapping.SAVE:
        stdinCommand = `${payload.command} ${payload.filePath}`;
        this.currentMinskyModelFilePath = payload.filePath;
        ipcMain.emit(events.ADD_RECENT_FILE, null, payload.filePath);
        break;

      case commandsMapping.MOUSEMOVE_SUBCOMMAND:
        stdinCommand = `${this.currentTab}/${payload.command} [${payload.mouseX}, ${payload.mouseY}]`;
        break;

      case commandsMapping.MOVE_TO:
        stdinCommand = `${payload.command} [${payload.mouseX}, ${payload.mouseY}]`;
        break;

      case commandsMapping.MOVE_TO_SUBCOMMAND:
        stdinCommand = `${this.currentTab}/${payload.command} [${payload.mouseX}, ${payload.mouseY}]`;
        break;

      case commandsMapping.MOUSEDOWN_SUBCOMMAND:
        // eslint-disable-next-line no-case-declarations
        const actualMouseDownCmd =
          this.currentTab === MainRenderingTabs.canvas
            ? payload.command
            : commandsMapping.MOUSEDOWN_FOR_OTHER_TABS;

        stdinCommand = `${this.currentTab}/${actualMouseDownCmd} [${payload.mouseX}, ${payload.mouseY}]`;
        break;

      case commandsMapping.MOUSEUP_SUBCOMMAND:
        stdinCommand = `${this.currentTab}/${payload.command} [${payload.mouseX}, ${payload.mouseY}]`;
        break;

      case commandsMapping.ZOOM_IN:
        stdinCommand = `${this.currentTab}/zoom [${payload.args.x}, ${payload.args.y}, ${payload.args.zoomFactor}]`;
        break;

      case commandsMapping.SET_GODLEY_ICON_RESOURCE:
        // eslint-disable-next-line no-case-declarations
        const godleyIconFilePath = normalizeFilePathForPlatform(
          Utility.isDevelopmentMode()
            ? `${join(__dirname, 'assets/godley.svg')}`
            : `${join(process.resourcesPath, 'assets/godley.svg')}`
        );
        stdinCommand = `${payload.command} ${godleyIconFilePath}`;

        break;

      case commandsMapping.SET_GROUP_ICON_RESOURCE:
        // eslint-disable-next-line no-case-declarations
        const groupIconFilePath = normalizeFilePathForPlatform(
          Utility.isDevelopmentMode()
            ? `${join(__dirname, 'assets/group.svg')}`
            : `${join(process.resourcesPath, 'assets/group.svg')}`
        );

        stdinCommand = `${payload.command} ${groupIconFilePath}`;
        break;

      case commandsMapping.SET_LOCK_ICON_RESOURCE:
        // eslint-disable-next-line no-case-declarations
        const lockIconFilePath = normalizeFilePathForPlatform(
          Utility.isDevelopmentMode()
            ? `${join(__dirname, 'assets/locked.svg')}`
            : `${join(process.resourcesPath, 'assets/locked.svg')}`
        );

        // eslint-disable-next-line no-case-declarations
        const unlockIconFilePath = normalizeFilePathForPlatform(
          Utility.isDevelopmentMode()
            ? `${join(__dirname, 'assets/unlocked.svg')}`
            : `${join(process.resourcesPath, 'assets/unlocked.svg')}`
        );

        stdinCommand = `${payload.command} [${lockIconFilePath},${unlockIconFilePath}]`;
        break;

      case commandsMapping.REQUEST_REDRAW_SUBCOMMAND:
        stdinCommand = this.getRequestRedrawCommand();
        break;

      default:
        stdinCommand = payload.command;
        break;
    }
    if (stdinCommand) {
      if (RecordingManager.isRecording) {
        RecordingManager.record(stdinCommand);
      }

      if (payload.command === commandsMapping.RENDER_FRAME_SUBCOMMAND) {
        // Render called explicitly
        this.render = false;
        callRESTApi(this.getRenderCommand());
        return callRESTApi(this.getRequestRedrawCommand());
        // TODO:: Check which of the above command's response we should return
      }

      const res = callRESTApi(stdinCommand);
      const { render = true } = payload;

      if ((USE_FRONTEND_DRIVEN_RENDERING && render) || this.render) {
        // TODO:: Let us retire unused flags and clean the flow
        const renderCommand = this.getRenderCommand();
        if (renderCommand) {
          callRESTApi(this.getRenderCommand());
          callRESTApi(this.getRequestRedrawCommand());
          this.render = false;
        }
      }
      return res;
    }
    log.error('Command was null or undefined');
  }

  private static getRequestRedrawCommand(tab?: MainRenderingTabs) {
    if (!tab) {
      tab = this.currentTab;
    }
    return `${tab}/${commandsMapping.REQUEST_REDRAW_SUBCOMMAND}`;
  }

  private static getRenderCommand(tab?: MainRenderingTabs) {
    const {
      leftOffset,
      canvasWidth,
      canvasHeight,
      activeWindows,
      electronTopOffset,
      scaleFactor,
    } = WindowManager;

    if (!tab) {
      tab = this.currentTab;
    }

    log.info('canvasHeight=', canvasHeight, ' canvasWidth=', canvasWidth);

    if (!canvasHeight || !canvasWidth) {
      return null;
    }

    const mainWindowId = activeWindows.get(1).systemWindowId;
    const renderCommand = `${tab}/${
      commandsMapping.RENDER_FRAME_SUBCOMMAND
    } [${mainWindowId},${leftOffset},${electronTopOffset},${canvasWidth},${canvasHeight}, ${scaleFactor.toPrecision(
      5
    )}]`; // TODO:: Remove this and fix backend to accept integer values
    return renderCommand;
  }

  static async setGodleyPreferences() {
    const preferences: MinskyPreferences = StoreManager.store.get(
      'preferences'
    );
    await this.handleMinskyProcess({
      command: `${commandsMapping.MULTIPLE_EQUITIES} ${preferences.enableMultipleEquityColumns}`,
    });
    await this.handleMinskyProcess({
      command: `${commandsMapping.SET_GODLEY_DISPLAY_VALUE} [${preferences.godleyTableShowValues},"${preferences.godleyTableOutputStyle}"]`,
    });
  }

  static async startMinskyService(showServiceStartedDialog = false) {
    const scope = this;
    const initPayload: MinskyProcessPayload = {
      command: commandsMapping.START_MINSKY_PROCESS,
      showServiceStartedDialog,
      render: false,
    };

    await scope.handleMinskyProcess(initPayload);

    const setGroupIconResource = async () => {
      const groupIconResourcePayload: MinskyProcessPayload = {
        command: commandsMapping.SET_GROUP_ICON_RESOURCE,
        render: false,
      };

      await scope.handleMinskyProcess(groupIconResourcePayload);
    };

    const setGodleyIconResource = async () => {
      const godleyIconPayload: MinskyProcessPayload = {
        command: commandsMapping.SET_GODLEY_ICON_RESOURCE,
        render: false,
      };

      await scope.handleMinskyProcess(godleyIconPayload);
    };

    const setLockIconResource = async () => {
      const lockIconPayload: MinskyProcessPayload = {
        command: commandsMapping.SET_LOCK_ICON_RESOURCE,
        render: false,
      };

      await scope.handleMinskyProcess(lockIconPayload);
    };

    setTimeout(async () => {
      await setGodleyIconResource();
      await setGroupIconResource();
      await setLockIconResource();
      await scope.setGodleyPreferences();
    }, 100);
  }
}
