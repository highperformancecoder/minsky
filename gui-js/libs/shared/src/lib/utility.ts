export class Utility {
  public static isDevelopmentMode(): boolean {
    const isEnvironmentSet: boolean = 'ELECTRON_IS_DEV' in process.env;
    const getFromEnvironment: boolean =
      parseInt(process.env.ELECTRON_IS_DEV, 10) === 1;

    return isEnvironmentSet ? getFromEnvironment : !Utility.isPackaged();
  }

  public static isPackaged(): boolean {
    // https://github.com/ganeshrvel/npm-electron-is-packaged/blob/master/lib/index.js

    let isPackaged = false;

    if (
      process.mainModule &&
      process.mainModule.filename.indexOf('app.asar') !== -1
    ) {
      isPackaged = true;
    } else if (
      process.argv.filter((a) => a.indexOf('app.asar') !== -1).length > 0
    ) {
      isPackaged = true;
    }

    return isPackaged;
  }
}
