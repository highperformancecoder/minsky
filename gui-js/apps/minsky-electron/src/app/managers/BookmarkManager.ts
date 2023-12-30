import { minsky } from '@minsky/shared';
import { Menu, MenuItem } from 'electron';
import { CommandsManager } from './CommandsManager';
import { WindowManager } from './WindowManager';

export class BookmarkManager {
  static async populateBookmarks(bookmarks: string[]) {
    const mainSubmenu = Menu.getApplicationMenu().getMenuItemById(
      'main-bookmark'
    ).submenu;

    const deleteBookmarkSubmenu = Menu.getApplicationMenu()
      .getMenuItemById('main-bookmark')
      .submenu.getMenuItemById('delete-bookmark').submenu;

    //const disableAllBookmarksInListAndDelete = () => {
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
    //};

    //const addNewBookmarks = () => {
      if (bookmarks.length) {
        bookmarks.forEach((bookmark, index) => {
          mainSubmenu.append(
            new MenuItem({
              id: 'minsky-bookmark',
              label: bookmark,
              click: async () => {
                minsky.model.gotoBookmark(index);
                WindowManager.getMainWindow().webContents.send('reset-scroll');
                await CommandsManager.requestRedraw();
              },
            })
          );

          deleteBookmarkSubmenu.append(
            new MenuItem({
              id: 'minsky-bookmark',
              label: bookmark,
              click: async () => {
                minsky.model.deleteBookmark(index);
                
                const _bookmarks = await minsky.model.bookmarkList();
                await CommandsManager.requestRedraw();

                await this.populateBookmarks(_bookmarks as string[]);
              },
            })
          );
        });
      }
    //};

    //disableAllBookmarksInListAndDelete();
    //addNewBookmarks();
  }

  static async updateBookmarkList() {
    let bookmarks=await minsky.canvas.model.bookmarkList();
    if (bookmarks!==null) // TODO why is this sometime null?
      this.populateBookmarks(bookmarks);
  }
}
