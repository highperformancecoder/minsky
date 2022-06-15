import { Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { StoreManager } from './StoreManager';

export class RecentFilesManager {
  static addFileToRecentFiles(filepath: string) {
    const recentFiles = StoreManager.store.get('recentFiles');

    const exists = Boolean(recentFiles.find((f) => f === filepath));
    if (!exists) {
      recentFiles.push(filepath);

      StoreManager.store.set('recentFiles', recentFiles);
    }
  }

  static initRecentFiles() {
    const recentFiles = StoreManager.store.get('recentFiles');

    const newRecentFiles = this.getRecentFilesThatMatchesRecentFilesToDisplay(
      recentFiles
    );

    if (recentFiles.length !== newRecentFiles.length) {
      StoreManager.store.set('recentFiles', newRecentFiles);
    }

    this.updateRecentFilesMenu(newRecentFiles);
  }

  private static updateRecentFilesMenu(newRecentFiles: string[]) {
    const openRecentMenu = Menu.getApplicationMenu().getMenuItemById(
      'openRecent'
    );

    openRecentMenu.submenu.items.forEach((f) => {
      if (f.id !== 'clearRecent') {
        f.visible = false;
      }
    });

    newRecentFiles.forEach((filePath) => {
      const position = 0;
      openRecentMenu.submenu.insert(
        position,
        new MenuItem({
          label: filePath,
          click: async () => {
            await CommandsManager.openNamedFile(filePath);
          },
        })
      );
    });
  }

  private static getRecentFilesThatMatchesRecentFilesToDisplay(
    recentFiles: string[]
  ) {
    const numberOfRecentFiles = recentFiles.length;

    const numberOfRecentFilesToDisplay = StoreManager.store.get('preferences')
      .numberOfRecentFilesToDisplay;

    if (
      numberOfRecentFiles &&
      numberOfRecentFiles > numberOfRecentFilesToDisplay
    ) {
      const numberOfItemsToBeRemoved =
        numberOfRecentFiles - numberOfRecentFilesToDisplay;

      if (numberOfItemsToBeRemoved > 0) {
        const newRecentFiles = recentFiles.slice(numberOfItemsToBeRemoved);

        return newRecentFiles;
      }
    }

    return recentFiles;
  }

  static updateNumberOfRecentFilesToDisplay() {
    const recentFiles = StoreManager.store.get('recentFiles');

    const newRecentFiles = this.getRecentFilesThatMatchesRecentFilesToDisplay(
      recentFiles
    );

    StoreManager.store.set('recentFiles', newRecentFiles);
  }
}
