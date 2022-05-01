import {
  availableOperations,
  commandsMapping,
  isEmptyObject,
  MainRenderingTabs,
  MinskyProcessPayload,
  ZOOM_IN_FACTOR,
  ZOOM_OUT_FACTOR,
} from '@minsky/shared';
import * as KeysymMapper from 'keysym';
import * as utf8 from 'utf8';
import { CommandsManager } from './CommandsManager';
import { RestServiceManager } from './RestServiceManager';

export class KeyBindingsManager {
  static async handleOnKeyPress(
    payload: MinskyProcessPayload
  ): Promise<unknown> {
    const {
      shift,
      capsLock,
      ctrl,
      alt,
      mouseX,
      mouseY,
      location,
      command = '',
    } = payload;
    let { key } = payload;

    if (shift && key === 'Tab') {
      key = 'BackTab';
    }
    const keySymAndName = this.getX11KeysymAndName(key, location);
    const _utf8 = this.getUtf8(key, ctrl);

    let modifierKeyCode = 0;
    if (shift) {
      modifierKeyCode += 1;
    }
    if (capsLock) {
      modifierKeyCode += 2;
    }
    if (ctrl) {
      modifierKeyCode += 4;
    }
    if (alt) {
      modifierKeyCode += 8;
    }

    const currentTab = RestServiceManager.getCurrentTab();

    if (keySymAndName.keysym) {
      const _payload: MinskyProcessPayload = {};
      // For godley popup, command sent by frontend is non-empty. It is the item accesor
      _payload.command = command
        ? `${command}/keyPress [${keySymAndName.keysym},"${_utf8}",${modifierKeyCode},${mouseX},${mouseY}]`
        : `${currentTab}/${commandsMapping.KEY_PRESS_COMMON_SUBCOMMAND} [${keySymAndName.keysym},"${_utf8}",${modifierKeyCode},${mouseX},${mouseY}]`;

      const isKeyPressHandled = await RestServiceManager.handleMinskyProcess(
        _payload
      );

      if (
        !isKeyPressHandled &&
        !command &&
        currentTab === MainRenderingTabs.canvas
      ) {
        return await this.handleOnKeyPressFallback(payload);
      }
      return isKeyPressHandled;
    }

    let res: boolean | string = false;
    if (!command && currentTab === MainRenderingTabs.canvas) {
      res = await this.handleOnKeyPressFallback(payload);
    }
    return res;
  }

  private static getUtf8(keyName: string, ctrl: boolean) {
    let _utf8 = null;
    if (ctrl) {
      const lowerCharCodeRange = {
        min: 'a'.charCodeAt(0),
        max: 'z'.charCodeAt(0),
      };
      const upperCharCodeRange = {
        min: 'A'.charCodeAt(0),
        max: 'Z'.charCodeAt(0),
      };

      let keyCharCode = keyName.charCodeAt(0);
      if (
        keyCharCode >= lowerCharCodeRange.min &&
        keyCharCode <= lowerCharCodeRange.max
      ) {
        keyCharCode = keyCharCode - lowerCharCodeRange.min + 1;
      } else if (
        keyCharCode >= upperCharCodeRange.min &&
        keyCharCode <= upperCharCodeRange.max
      ) {
        keyCharCode = keyCharCode - upperCharCodeRange.min + 1;
      }
      _utf8 = String.fromCharCode(keyCharCode);
    }
    if (!_utf8) {
      _utf8 = utf8.encode(keyName); // TODO:: Review - do we need this?
    }
    return _utf8;
  }

  private static getX11KeysymAndName(keyName: string, location: number) {
    // Returns X11 Keysym and a mapped name. Events that we need are especially handled.
    // Keysyms needed in Minsky: https://gitlab.com/varun_coditas/minsky-angular/-/issues/174
    // Backspace, Escape, Return, KP_Enter, 4 arrow keys, Tab, BackTab, Page Up, Page Down

    const JSToX11KeynameMap = {
      ArrowLeft: { 0: 'Left' },
      ArrowUp: { 0: 'Up' },
      ArrowRight: { 0: 'Right' },
      ArrowDown: { 0: 'Down' },
      Shift: { 1: 'Shift_L', 2: 'Shift_R' },
      Enter: { 0: 'Return', 3: 'KP_Enter' },
    };

    /* The numeric parameter in the map above is "location" returned by Javascript */
    // location:: https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent
    // DOM_KEY_LOCATION_STANDARD -> 0
    // DOM_KEY_LOCATION_LEFT -> 1,
    // DOM_KEY_LOCATION_RIGHT -> 2,
    // DOM_KEY_LOCATION_NUMPAD -> 3

    const renameKeyOptions = JSToX11KeynameMap[keyName];
    if (renameKeyOptions) {
      if (location in renameKeyOptions) {
        keyName = renameKeyOptions[location];
      }
    }

    /* Taken from https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h */
    const customKeysymMap = {
      Backspace: 0xff08,
      Left: 0xff51,
      Up: 0xff52,
      Right: 0xff53,
      Down: 0xff54,
      PageUp: 0xff55,
      PageDown: 0xff56,
      Tab: 0xff09,
      BackTab: 0xfd05,
      Escape: 0xff1b,
      Return: 0xff0d,
      KP_Enter: 0xff8d,
      Shift_L: 0xffe1,
      Shift_R: 0xffe2,
    };

    // We first lookup in our customKeysymMap, else fallback to what keysym library provides
    let _keysym = customKeysymMap[keyName];
    if (!_keysym) {
      _keysym = KeysymMapper.fromName(keyName)?.keysym;
    }
    return {
      keysym: _keysym,
      name: keyName,
    };
  }

  private static isCtrlPayload(payload: MinskyProcessPayload) {
    return payload.ctrl && !payload.alt && !payload.shift;
  }

  private static async handleOnKeyPressFallback(payload: MinskyProcessPayload) {
    let executed = true;
    const { key } = payload;

    switch (key) {
      case 'Backspace':
      case 'Delete':
        await this.deleteKey(payload);
        break;

      case '+':
        await this.handlePlusKey(payload);
        break;

      case '-':
        if (payload.location !== 3) {
          executed = false;
        } else {
          await this.handleMinusKey(payload);
        }
        break;

      case '*':
        await CommandsManager.addOperation(availableOperations.MULTIPLY);
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case '/':
        await CommandsManager.addOperation(availableOperations.DIVIDE);
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case '^':
        await CommandsManager.addOperation(availableOperations.POW);
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case '&':
        await CommandsManager.addOperation(availableOperations.INTEGRATE);
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case '=':
        await CommandsManager.addGodley();
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case '@':
        await CommandsManager.addPlot();
        await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);

        break;

      case 'x':
      case 'X':
        if (this.isCtrlPayload(payload)) {
          await CommandsManager.cut();
        } else {
          executed = false;
        }

        break;

      case 'c':
      case 'C':
        if (this.isCtrlPayload(payload)) {
          await CommandsManager.copy();
        } else {
          executed = false;
        }

        break;

      case 'v':
      case 'V':
        if (this.isCtrlPayload(payload)) {
          await CommandsManager.paste();
        } else {
          executed = false;
        }

        break;

      default:
        executed = false;
        break;
    }

    if (payload.ctrl) {
      // avoiding conflict with shortCuts (electron accelerators)
      return;
    }

    return executed;
  }

  private static async handlePlusKey(payload: MinskyProcessPayload) {
    if (payload.shift) {
      // <Key-plus>
      await CommandsManager.addOperation(availableOperations.ADD);
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      return;
    }

    // <Key-KP_Add>
    await this.zoom(ZOOM_IN_FACTOR);
    return;
  }

  private static async handleMinusKey(payload: MinskyProcessPayload) {
    // <Key-KP_Subtract>
    await this.zoom(ZOOM_OUT_FACTOR);
    return;
  }

  private static async zoom(factor: number) {
    const cBounds = await RestServiceManager.handleMinskyProcess({
      command: commandsMapping.C_BOUNDS,
    });

    const x = 0.5 * (cBounds[2] + cBounds[0]);
    const y = 0.5 * (cBounds[3] + cBounds[1]);

    this.zoomAt(x, y, factor);
    return;
  }

  private static async zoomAt(x: number, y: number, zoomFactor: number) {
    await RestServiceManager.handleMinskyProcess({
      command: `${commandsMapping.ZOOM_IN} [${x},${y},${zoomFactor}]`,
    });

    await CommandsManager.requestRedraw();
    return;
  }

  private static async deleteKey(payload: MinskyProcessPayload) {
    const { mouseX, mouseY } = payload;

    const isCanvasSelectionEmpty = (await RestServiceManager.handleMinskyProcess(
      {
        command: commandsMapping.CANVAS_SELECTION_EMPTY,
      }
    )) as boolean;

    if (!isCanvasSelectionEmpty) {
      await RestServiceManager.handleMinskyProcess({
        command: commandsMapping.CUT,
      });
      return;
    }

    const item = await CommandsManager.getItemAt(mouseX, mouseY);
    if (!isEmptyObject(item)) {
      const itemId = await CommandsManager.getCurrentItemId();
      await CommandsManager.deleteCurrentItemHavingId(itemId);
      return;
    }
    const wire = await CommandsManager.getWireAt(mouseX, mouseY);
    if (!isEmptyObject(wire)) {
      await RestServiceManager.handleMinskyProcess({
        command: commandsMapping.CANVAS_DELETE_WIRE,
      });
      return;
    }
  }
}
