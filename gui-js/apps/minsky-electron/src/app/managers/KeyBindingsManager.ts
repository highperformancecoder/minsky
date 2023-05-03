import {
  MinskyProcessPayload,
  ZOOM_OUT_FACTOR,
  minsky, RenderNativeWindow,
} from '@minsky/shared';
import * as utf8 from 'utf8';
import { CommandsManager } from './CommandsManager';
import { WindowManager } from './WindowManager';
import * as JSON5 from 'json5';

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

    const currentTab = WindowManager.currentTab;

    if (keySymAndName.keysym) {
      // For godley popup, command sent by frontend is non-empty. It is the item accesor
      let renderer=command? new RenderNativeWindow(command): currentTab;
      const isKeyPressHandled = renderer.keyPress(
        {
          keySym:keySymAndName.keysym,
          utf8: _utf8,
          state: modifierKeyCode,
          x: mouseX,
          y: mouseY
        });
      
      if (
        !isKeyPressHandled &&
          !command &&
          currentTab.equal(minsky.canvas)
      ) {
        return await this.handleOnKeyPressFallback({key:keySymAndName.name, ctrl, mouseX, mouseY});
      }
      return isKeyPressHandled;
    }

    let res: boolean | string = false;
    if (!command && currentTab.equal(minsky.canvas)) {
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
            BackTab: 0xfe20,
            Escape: 0xff1b,
            Return: 0xff0d,
            KP_Enter: 0xff8d,
            Shift_L: 0xffe1,
            Shift_R: 0xffe2,
        };
        
        let _keysym = customKeysymMap[keyName];

        if (!_keysym && keyName.length==1) {
            // we make the assumption that UTF16 characters map
            // verbatim to XKeySyms - only a problem for UTF16
            // characters in the range 0xff00 to 0xffff.
          _keysym = keyName.charCodeAt(0);
        }
      console.log(`keyName=${keyName}, keySym=${_keysym}\n`);
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
      minsky.canvas.addOperation("multiply");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;

    case '/':
      minsky.canvas.addOperation("divide");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;

    case '^':
      minsky.canvas.addOperation("pow");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;
      
    case '%':
      minsky.canvas.addOperation("percent");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;

    case '&':
      minsky.canvas.addOperation("integrate");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;

    case '=':
      minsky.canvas.addGodley();
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      break;

    case '@':
      minsky.canvas.addPlot();
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

    case 'F1':
      CommandsManager.help(payload.mouseX, payload.mouseY);
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
//    if (payload.shift) {
      // <Key-plus>
      minsky.canvas.addOperation("add");
      await CommandsManager.mouseUp(payload.mouseX, payload.mouseY);
      return;
//    }

//    // <Key-KP_Add>
//    await this.zoom(ZOOM_IN_FACTOR);
//    return;
  }

  private static async handleMinusKey(payload: MinskyProcessPayload) {
    // <Key-KP_Subtract>
    await this.zoom(ZOOM_OUT_FACTOR);
    return;
  }

  private static async zoom(factor: number) {
    const cBounds = minsky.model.cBounds();
    const x = 0.5 * (cBounds[2] + cBounds[0]);
    const y = 0.5 * (cBounds[3] + cBounds[1]);

    this.zoomAt(x, y, factor);
    return;
  }

  private static async zoomAt(x: number, y: number, zoomFactor: number) {
    minsky.canvas.zoom(x,y,zoomFactor);
    await CommandsManager.requestRedraw();
    return;
  }

  private static async deleteKey(payload: MinskyProcessPayload) {
    const { mouseX, mouseY } = payload;

    if (!minsky.canvas.selection.empty()) {
      minsky.cut();
      return;
    }

    if (minsky.canvas.getItemAt(mouseX, mouseY)) {
      await CommandsManager.deleteCurrentItemHavingId(minsky.canvas.item.id());
      return;
    }
    if (minsky.canvas.getWireAt(mouseX, mouseY)) {
      minsky.canvas.deleteWire();
      return;
    }
  }
}
