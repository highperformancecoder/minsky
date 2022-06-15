import { commandsMapping } from '@minsky/shared';
import { Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { RestServiceManager } from './RestServiceManager';

export class BookmarkManager {
  static async populateBookmarks(bookmarks: string[]) {
    const mainSubmenu = Menu.getApplicationMenu().getMenuItemById(
      'main-bookmark'
    ).submenu;

    const deleteBookmarkSubmenu = Menu.getApplicationMenu()
      .getMenuItemById('main-bookmark')
      .submenu.getMenuItemById('delete-bookmark').submenu;

    const disableAllBookmarksInListAndDelete = () => {
      mainSubmenu.items.forEach((bookmark) => {
        if (bookmark.id === 'minsky-bookmark') {
          bookmark.visible = false;
        }
      });

      deleteBookmarkSubmenu.items.forEach((bookmark) => {
        if (bookmark.id === 'minsky-bookmark') {
          bookmark.visible = false;
        }
      });
    };

    const addNewBookmarks = () => {
      if (bookmarks.length) {
        bookmarks.forEach((bookmark, index) => {
          mainSubmenu.append(
            new MenuItem({
              id: 'minsky-bookmark',
              label: bookmark,
              click: async () => {
                await RestServiceManager.handleMinskyProcess({
                  command: `${commandsMapping.GOTO_BOOKMARK} ${index}`,
                });
                await CommandsManager.requestRedraw();
              },
            })
          );

          deleteBookmarkSubmenu.append(
            new MenuItem({
              id: 'minsky-bookmark',
              label: bookmark,
              click: async () => {
                await RestServiceManager.handleMinskyProcess({
                  command: `${commandsMapping.DELETE_BOOKMARK} ${index}`,
                });

                const _bookmarks = await RestServiceManager.handleMinskyProcess(
                  {
                    command: commandsMapping.BOOKMARK_LIST,
                  }
                );
                await CommandsManager.requestRedraw();

                await this.populateBookmarks(_bookmarks as string[]);
              },
            })
          );
        });
      }
    };

    disableAllBookmarksInListAndDelete();
    addNewBookmarks();
  }
}
