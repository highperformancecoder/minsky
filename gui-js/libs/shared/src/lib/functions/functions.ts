declare var process;

export class Functions {
  static green(anything: unknown) {
    return '\x1b[32m' + `${anything}`;
  }
  
  static red(anything: unknown) {
    return '\x1b[31m' + `${anything}`;
  }
  
  static getBackgroundStyle(color) {
    // Variables for red, green, blue values
  
    let colorArray;
    let r, g, b;
    // Check the format of the color, HEX or RGB?
    if (color.match(/^rgb/)) {
      // If RGB --> store the red, green, blue values in separate variables
      colorArray = color.match(
        /^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+(?:\.\d+)?))?\)$/
      );
  
      r = colorArray[1];
      g = colorArray[2];
      b = colorArray[3];
    } else {
      // If hex --> Convert it to RGB: http://gist.github.com/983661
      colorArray = +(
        '0x' + color.slice(1).replace(color.length < 5 && /./g, '$&$&')
      );
  
      r = colorArray >> 16;
      g = (colorArray >> 8) & 255;
      b = colorArray & 255;
    }
  
    // HSP (Highly Sensitive Poo) equation from http://alienryderflex.com/hsp.html
    const hsp = Math.sqrt(0.299 * (r * r) + 0.587 * (g * g) + 0.114 * (b * b));
  
    // Using the HSP value, determine whether the color is light or dark
    if (hsp > 127.5) {
      return {
        style: 'body { background-color: ' + color + '; color: black; }',
        r,
        g,
        b,
      };
    }
  
    return {
      style: 'body { background-color: ' + color + '; color: white; }',
      r,
      g,
      b,
    };
  }

  static isWindows() {
    try {
      // can throw "process is not defined" even when we check if it's undefined...
      return process?.platform === 'win32';
    } catch {
      return false;
    }
  }
  
  static isMacOS() {
    try {
      // can throw "process is not defined" even when we check if it's undefined...
      return process?.platform === 'darwin';
    } catch {
      return false;
    }
  }
}

