import { Injectable } from '@angular/core';
import { MatDialog, MatDialogRef } from '@angular/material/dialog';
import {
  AppLayoutPayload,
  Canvas,
  CppClass,
  events,
  EventInterface,
  HeaderEvent,
  importCSVvariableName,
  MainRenderingTabs,
  MinskyProcessPayload,
  PubTab,
  ReplayRecordingStatus,
  TypeValueName,
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR,
} from '@minsky/shared';
import { BehaviorSubject } from 'rxjs';
import { WindowUtilityService } from '../WindowUtility/window-utility.service';
import { DialogComponent } from './../../component/dialog/dialog.component';
import { ElectronService } from './../electron/electron.service';
import JSON5 from 'json5';

export class Message {
  id: string;
  body: string;
}

interface ReplayJSON {
  command: string;
  executedAt: number;
}

@Injectable({
  providedIn: 'root',
})
export class CommunicationService {
  private scrollPositionAtMouseDown: {
    x: number;
    y: number;
  };
  private mousePositionAtMouseDown: {
    x: number;
    y: number;
  };

  currentTab = MainRenderingTabs.canvas as string;
  showPlayButton$ = new BehaviorSubject<boolean>(true);
  t = '0';
  deltaT = '0';

  mouseX: number;
  mouseY: number;

  awaitingZoom = false; // flag to avoid backing up zoom events

  drag = false;
  currentReplayJSON: ReplayJSON[] = [];

  ReplayRecordingStatus$: BehaviorSubject<ReplayRecordingStatus> = new BehaviorSubject(
    ReplayRecordingStatus.ReplayStopped
  );

  delay = 0;
  runUntilTime: number;

  resetScrollWhenIdle: any;


  private dialogRef: MatDialogRef<DialogComponent, any> = null;
  availableOperations = null;

  resetScroll = () => { }; // callback to reset canvas scrollbars

  constructor(
    // private socket: Socket,
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService,
    private dialog: MatDialog
  ) {
    this.scrollPositionAtMouseDown = null;
    this.mousePositionAtMouseDown = null;
    this.initReplay();
    this.electronService.on('reset-scroll', async () => { this.resetScroll(); });
  }

  private async syncRunUntilTime() {
    this.runUntilTime = await this.electronService.minsky.tmax();
  }

  private initReplay() {
    if (this.electronService.isElectron) {
      this.electronService.on(
        events.REPLAY_RECORDING,
        async (event, { json }) => {
          this.ReplayRecordingStatus$.next(ReplayRecordingStatus.ReplayStarted);
          this.currentReplayJSON = json;
          this.showPlayButton$.next(false);

          await this.electronService.invoke(events.NEW_SYSTEM);
          this.startReplay();
        }
      );
    }
  }

  replayNextCommand() {
    const { command: commandArgs } = this.currentReplayJSON.shift();
    const sep = commandArgs.trim().indexOf(' ');
    const command = commandArgs.substring(0, sep);
    if (sep === -1)
      CppClass.backend(command);
    else {
      const args = JSON5.parse(commandArgs.substring(sep));
      CppClass.backend(command, args);
    }
  }

  startReplay() {
    setTimeout(async () => {
      if (!this.currentReplayJSON.length) {
        this.ReplayRecordingStatus$.next(ReplayRecordingStatus.ReplayStopped);
        this.showPlayButton$.next(true);
        return;
      }

      this.replayNextCommand();
      if (
        this.ReplayRecordingStatus$.value ===
        ReplayRecordingStatus.ReplayStarted
      ) {
        this.startReplay();
      }
    }, this.delay || 1);
  }

  stopReplay() {
    this.currentReplayJSON = [];
    this.ReplayRecordingStatus$.next(ReplayRecordingStatus.ReplayStopped);
  }

  pauseReplay() {
    this.ReplayRecordingStatus$.next(ReplayRecordingStatus.ReplayPaused);
  }

  continueReplay() {
    this.ReplayRecordingStatus$.next(ReplayRecordingStatus.ReplayStarted);
    this.startReplay();
  }

  async stepReplay() {
    if (!this.currentReplayJSON.length) {
      return;
    }

    this.replayNextCommand();
  }

  setBackgroundColor(color = null) {
    if (this.electronService.isElectron)
      this.electronService.send(events.SET_BACKGROUND_COLOR, {
        color: color,
      });
  }

  public async sendEvent(event: string, message: HeaderEvent) {
    try {
      const { target } = message;
      if (this.electronService.isElectron) {
        const dimensions = this.windowUtilityService.getDrawableArea();
        const canvasWidth = dimensions.width;
        const canvasHeight = dimensions.height;
        let minsky = this.electronService.minsky;
        let currentTab = new EventInterface(this.currentTab);
        switch (target) {
          case 'ZOOM_OUT':
            await currentTab.zoom(canvasWidth / 2, canvasHeight / 2, ZOOM_OUT_FACTOR);
            this.resetScroll();
            break;
          case 'ZOOM_IN':
            await currentTab.zoom(canvasWidth / 2, canvasHeight / 2, ZOOM_IN_FACTOR);
            this.resetScroll();
            break;
          case 'RESET_ZOOM':
            await this.resetZoom(canvasWidth / 2, canvasHeight / 2);
            this.resetScroll();
            break;
          case 'ZOOM_TO_FIT':
            await this.zoomToFit(canvasWidth, canvasHeight);
            this.resetScroll();
            break;
          case 'SIMULATION_SPEED':
            await this.updateSimulationSpeed(message);
            break;
          case 'PLAY':
            this.currentReplayJSON.length
              ? this.continueReplay()
              : this.initSimulation();

            break;
          case 'PAUSE':
            this.currentReplayJSON.length
              ? this.pauseReplay()
              : await this.pauseSimulation();
            break;
          case 'RESET':
            this.showPlayButton$.next(true);
            this.currentReplayJSON.length
              ? this.stopReplay()
              : await this.stopSimulation();
            break;
          case 'STEP':
            this.currentReplayJSON.length
              ? this.stepReplay()
              : await this.stepSimulation();
            break;
          case 'REVERSE_CHECKBOX':
            minsky.reverse(message.value as boolean);
            break;
          case 'RECORD':
            this.electronService.record();
            break;
          case 'RECORDING_REPLAY':
            this.electronService.recordingReplay();
            break;
          default:
            break;
        }
      }
    } catch (error) {
      console.error(
        '🚀  file: communication.service.ts ~ line 188 ~ CommunicationService ~ sendEvent ~ error',
        error
      );
    }
  }

  private async updateSimulationSpeed(message: HeaderEvent) {
    const speed = message.value as number;

    const currentDelay: number = 12 - (speed * 12) / (150 - 0);

    this.delay = currentDelay ? Math.round(Math.pow(10, currentDelay / 4)) : 0;
  }

  private async stepSimulation() {
    const [t, deltaT] = (await this.electronService.minsky.step());

    this.updateSimulationTime(t, deltaT);
  }

  private async initSimulation() {
    await this.electronService.minsky.running(true);
    this.startSimulation();
  }

  private startSimulation() {
    this.syncRunUntilTime();
    this.simulate();
    this.electronService.minsky.dimensionalAnalysis();
  }

  private simulate() {
    setTimeout(async () => {
      if (!this.showPlayButton$.value) {
        const [t, deltaT] = await this.electronService.minsky.step();
        this.updateSimulationTime(t, deltaT);
        this.simulate();
      }
    }, this.delay);
  }

  private async pauseSimulation() {
    this.electronService.minsky.$callMethodSync('running',false);
  }

  private async stopSimulation() {
    await this.electronService.minsky.running(false);

    this.electronService.minsky.reset();
    this.electronService.minsky.dimensionalAnalysis();

    const t = await this.electronService.minsky.t();
    const deltaT = await this.electronService.minsky.deltaT();
    this.updateSimulationTime(t, deltaT);
  }

  private updateSimulationTime(t: number, deltaT: number) {

    this.t = t.toFixed(2);

    this.deltaT = deltaT.toFixed(2);

    if (Number(this.t) >= this.runUntilTime) {
      this.pauseSimulation();
    }
  }

  private async resetZoom(centerX: number, centerY: number) {
    if (this.currentTab === MainRenderingTabs.canvas) {
      let minsky = this.electronService.minsky;
      const zoomFactor = await minsky.canvas.model.zoomFactor();
      if (zoomFactor > 0) {
        const relZoom = await minsky.canvas.model.relZoom();

        //if relZoom = 0 ;use relZoom as 1 to avoid returning infinity
        minsky.canvas.zoom(centerX, centerY, 1 / (relZoom || 1));
      } else {
        minsky.canvas.model.setZoom(1);
      }

      minsky.canvas.recentre();
      minsky.canvas.requestRedraw();
    } else {
      let currentTab = new EventInterface(this.currentTab);
      const zoomFactor = await currentTab.zoomFactor();
      if (zoomFactor > 0)
        currentTab.zoom(centerX, centerY, 1 / zoomFactor);
    }
  }

  private async zoomToFit(canvasWidth: number, canvasHeight: number) {
    if (this.currentTab !== MainRenderingTabs.canvas) return;
    let minsky = this.electronService.minsky;
    const cBounds = await minsky.canvas.model.cBounds();

    const zoomFactorX = canvasWidth / (cBounds[2] - cBounds[0]);
    const zoomFactorY = canvasHeight / (cBounds[3] - cBounds[1]);

    const zoomFactor = Math.min(zoomFactorX, zoomFactorY);
    const x = 0.5 * (cBounds[2] + cBounds[0]);
    const y = 0.5 * (cBounds[3] + cBounds[1]);

    minsky.canvas.zoom(x, y, zoomFactor);
    minsky.canvas.recentre();
    minsky.canvas.requestRedraw();
  }

  public async mouseEvents(event, message: MouseEvent) {
    const { type, clientX, clientY, button } = message;
    const offset = this.windowUtilityService.getMinskyCanvasOffset();

    this.mouseX = clientX;
    this.mouseY = clientY - Math.round(offset.top);
    const yoffs = this.electronService.isMacOS() ? -172 : 0; // why, o why, Mac?

    if (event === 'contextmenu') {
      this.electronService.send(events.CONTEXT_MENU, {
        x: this.mouseX,
        y: this.mouseY + yoffs,
        type: "canvas",
      });
      return;
    }

    if (button === 2) {
      // on right click
      return;
    }

    if (this.electronService.isElectron) {

      if (type === 'mousedown' && message.altKey) {
        this.electronService.send(
          events.DISPLAY_MOUSE_COORDINATES,
          { mouseX: this.mouseX, mouseY: this.mouseY }
        );
        return;
      }

      // TODO:: Should the drag logic be in this branch or else? isElectron / FE?

      if (type === 'mousedown' && message.shiftKey) {
        this.drag = true;
        this.scrollPositionAtMouseDown = {
          x: this.windowUtilityService.getMinskyContainerElement().scrollLeft,
          y: this.windowUtilityService.getMinskyContainerElement().scrollTop,
        };
        this.mousePositionAtMouseDown = {
          x: this.mouseX,
          y: this.mouseY,
        };
        return;
      }

      if (type === 'mouseup' && this.drag) {
        this.drag = false;
        return;
      }

      if (type === 'mousemove' && this.drag) {
        const deltaX = this.mousePositionAtMouseDown.x - this.mouseX;
        const deltaY = this.mousePositionAtMouseDown.y - this.mouseY;

        this.windowUtilityService.getMinskyContainerElement().scrollTop =
          this.scrollPositionAtMouseDown.y + deltaY;

        this.windowUtilityService.getMinskyContainerElement().scrollLeft =
          this.scrollPositionAtMouseDown.x + deltaX;
        return;
      }

      let canvas = new Canvas(this.currentTab);

      switch (type) {

        case 'mousedown':
          canvas.mouseDown(clientX, this.mouseY + yoffs);
          break;
        case 'mouseup':
          canvas.mouseUp(clientX, this.mouseY + yoffs);
          break;
        case 'mousemove':
          canvas.mouseMove(clientX, this.mouseY + yoffs);
          break;
      }
    }
  }

  async setWindowSizeAndCanvasOffsets() {
    const isMainWindow = await this.windowUtilityService.isMainWindow();
    // Code for canvas offset values
    if (this.electronService.isElectron && isMainWindow) {
      await this.windowUtilityService.reInitialize();
      const offset = this.windowUtilityService.getMinskyCanvasOffset();
      const drawableArea = this.windowUtilityService.getDrawableArea();
      this.electronService.send(events.APP_LAYOUT_CHANGED, {
        offset: offset,
        drawableArea: drawableArea,
      } as AppLayoutPayload);
    }
  }

  async addOperation(arg) {
    if (this.electronService.isElectron) {
      this.electronService.minsky.canvas.addOperation(arg);
    }
  }

  async importData() {
    this.electronService.minsky.canvas.addVariable(importCSVvariableName, 'parameter');

    const payload: MinskyProcessPayload = {
      mouseX: await this.electronService.minsky.canvas.itemFocus.x(),
      mouseY: await this.electronService.minsky.canvas.itemFocus.y(),
    };
    this.electronService.invoke(events.IMPORT_CSV, payload);
  }

  resetScrollTimeout = () => {
    this.resetScroll();
    this.resetScrollWhenIdle = null;
  };

  onMouseWheelZoom = async (event: WheelEvent) => {
    event.preventDefault();
    const { deltaY } = event;
    const zoomIn = deltaY < 0;
    const offset = this.windowUtilityService.getMinskyCanvasOffset();

    const x = event.clientX - offset.left;
    const y = event.clientY - offset.top;
    let zoomFactor = null;
    if (zoomIn) {
      zoomFactor = ZOOM_IN_FACTOR;
    } else {
      zoomFactor = ZOOM_OUT_FACTOR;
    }

    if (this.awaitingZoom) return; // remove zoom events coming too fast
    this.awaitingZoom = true;
    let currentTab = new EventInterface(this.currentTab);
    await currentTab.zoom(x, y, zoomFactor);
    this.awaitingZoom = false;

    // schedule resetScroll when zooming stops
    if (this.resetScrollWhenIdle) clearTimeout(this.resetScrollWhenIdle);
    this.resetScrollWhenIdle = setTimeout(this.resetScrollTimeout, 100);
  };

  async handleKeyUp(event: KeyboardEvent) {
    if (!event.shiftKey) {
      this.drag = false;
    }
    return;
  }

  async handleKeyDown({
    event,
    command = '',
  }: {
    event: KeyboardEvent;
    command?: string;
  }) {
    if (
      [
        'ArrowRight',
        'ArrowLeft',
        'ArrowUp',
        'ArrowDown',
        'PageUp',
        'PageDown',
      ].includes(event.key) || (event.key == ' ' && !this.dialogRef)
    ) {
      // this is to prevent scroll events on press if arrow and page up/down keys
      event.preventDefault();
    }

    const isMainWindow = await this.windowUtilityService.isMainWindow();
    if (isMainWindow && (event.ctrlKey || event.metaKey) && (event.key.match("^[Noq]$")))
      return; // perform menu accelerator only

    if (
      isMainWindow &&
      ((this.dialogRef && event.ctrlKey) || (this.dialogRef && event.altKey) || (this.dialogRef && event.metaKey))
    ) {
      // return when dialog is open anything is pressed with ctrl or alt
      return;
    }

    const payload: MinskyProcessPayload = {
      command,
      key: event.key,
      shift: event.shiftKey,
      capsLock: event.getModifierState('CapsLock'),
      ctrl: event.ctrlKey,
      alt: event.altKey,
      mouseX: this.mouseX,
      mouseY: this.mouseY,
      location: event.location,
    };


    if (!isMainWindow) {
      await this.electronService.invoke(
        events.KEY_PRESS,
        {
          ...payload,
          command: payload.command.trim(),
        }
      );
      return;
    }

    if (!this.dialogRef) {
      const isKeyHandled = this.electronService.sendSync(
        events.KEY_PRESS,
        {
          ...payload,
          command: payload.command.trim(),
        }
      );

      const asciiRegex = /[ -~]/;

      if (
        !isKeyHandled &&
        event.key.length === 1 &&
        event.key.match(asciiRegex) &&
        !event.altKey &&
        !event.ctrlKey &&
        !event.metaKey
      ) {
        this.dialogRef = this.dialog.open(DialogComponent, {
          width: '600px',
          position: { top: '0', left: '33.33%' },
          data: { value: event.key },
        });
        this.dialogRef.componentInstance.setValue(event.key);

        this.dialogRef.afterClosed().subscribe(async (multipleKeyString) => {
          this.dialogRef = null;
          await this.handleTextInputSubmit(multipleKeyString);
        });
      }

      return;
    }
  }

  private showCreateVariablePopup(title: string, params: TypeValueName) {
    const urlParts = Object.keys(params)
      .map((pKey) => {
        return `${pKey}=${encodeURIComponent(params[pKey])}`;
      })
      .join('&');

    this.electronService.send(events.CREATE_MENU_POPUP, {
      width: 500,
      height: 650,
      title: title,
      url: `#/headless/menu/insert/create-variable?${urlParts}`,
    });
  }

  async handleTextInputSubmit(multipleKeyString: string) {
    switch (this.currentTab) {
    case 'minsky.canvas':
      if (this.electronService.isElectron && multipleKeyString) {
        if (multipleKeyString.charAt(0) === '#') {
          const note = multipleKeyString.slice(1);
          this.electronService.minsky.canvas.addNote(note);
          return;
        }

        if (multipleKeyString === '-') {
          this.addOperation('subtract');
          return;
        }

        if (!isNaN(Number(multipleKeyString))) {
          this.showCreateVariablePopup('Create Constant', {
            type: 'constant',
            value: multipleKeyString,
          });
          return;
        }

        const operations = await this.electronService.minsky.availableOperations();
        const operation = multipleKeyString.toLowerCase();
        
        if (operations.includes(operation)) {
          this.addOperation(operation);
          return;
        }
        
        const popupTitle = 'Specify Variable Name';
        const params: TypeValueName = {
          type: 'flow',
          name: multipleKeyString,
        };

        this.showCreateVariablePopup(popupTitle, params);
      }
      return;
    case 'minsky.equationDisplay':
    case 'minsky.phillipsDiagram':
      return; // do nothing
    default: // pub tabs
      new PubTab(this.currentTab).addNote(multipleKeyString, this.mouseX, this.mouseY);
      return;
    }
  }

  handleDblClick() {
    this.electronService.send(events.DOUBLE_CLICK, {
      mouseX: this.mouseX,
      mouseY: this.mouseY,
    });
  }

}
