import { defaultBackgroundColor } from '@minsky/shared';
import Store from 'electron-store';
import {homedir} from 'node:os';

interface MinskyPreferences {
  godleyTableShowValues: boolean;
  godleyTableOutputStyle: string;
  enableMultipleEquityColumns: boolean;
  numberOfRecentFilesToDisplay: number;
  wrapLongEquationsInLatexExport: boolean;
  font: string;
  numBackups: number;
}

interface MinskyStore {
  recentFiles: Array<string>;
  backgroundColor: string;
  preferences: MinskyPreferences;
  defaultModelDirectory: string;
  defaultDataDirectory: string;
  ravelPlugin: string; // used for post installation installation of Ravel
}

class StoreManager {
  // TODO:: We should sync the initial values with backend
  static store = new Store<MinskyStore>({
    defaults: {
      recentFiles: [],
      backgroundColor: defaultBackgroundColor,
      defaultModelDirectory: homedir(),
      defaultDataDirectory: homedir(),
      preferences: {
        godleyTableShowValues: false,
        godleyTableOutputStyle: 'sign',
        enableMultipleEquityColumns: false,
        numberOfRecentFilesToDisplay: 10,
        wrapLongEquationsInLatexExport: false,
        font: "",
        numBackups: 1,
      },
      ravelPlugin: '',
    },
  });
}

export { StoreManager, MinskyPreferences };
