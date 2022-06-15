export const isEmptyObject = (obj) => {
  return Object.keys(obj).length === 0 && obj.constructor === Object;
};

export const green = (anything: unknown): string => {
  return '\x1b[32m' + `${anything}`;
};

export const red = (anything: unknown): string => {
  return '\x1b[31m' + `${anything}`;
};

export const getBackgroundStyle = (color) => {
  // Variables for red, green, blue values

  let colorArray;
  let r, g, b;
  // Check the format of the color, HEX or RGB?
  if (color.match(/^rgb/)) {
    // If RGB --> store the red, green, blue values in separate variables
    colorArray = color.match(
      /^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+(?:\.\d+)?))?\)$/
    );

    r = color[1];
    g = color[2];
    b = color[3];
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
};

export const isWindows = () => process && process.platform === 'win32';
export const isMacOS = () => process && process.platform === 'darwin';

export const normalizeFilePathForPlatform = (filePath: string) => {
  // TODO:: Why do we need stringification? Does our backend API require it? Is it needed only on windows?
  // TODO:: Why not JSON5?
  if (filePath && filePath.charAt(0) !== '"') {
    return JSON.stringify(filePath);
  }
  return filePath;
};

export const replaceBackSlash = (str: string) => {
  if (str && typeof str === 'string') {
    return str.replace(/\\/g, '\\\\');
  }

  return str;
};
