import { Component } from '@angular/core';
import { FormControl, Validators } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { commandsMapping, events, replaceBackSlash } from '@minsky/shared';

@Component({
  selector: 'minsky-add-bookmark',
  templateUrl: './add-bookmark.component.html',
  styleUrls: ['./add-bookmark.component.scss'],
})
export class AddBookmarkComponent {
  bookmarkName: FormControl;

  constructor(private electronService: ElectronService) {
    this.bookmarkName = new FormControl('', Validators.required);
  }

  async handleSubmit() {
    await this.electronService.sendMinskyCommandAndRender({
      command: `${commandsMapping.ADD_BOOKMARK} "${replaceBackSlash(
        this.bookmarkName.value
      )}"`,
    });

    const bookmarks = (await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.BOOKMARK_LIST,
    })) as string[];

    this.electronService.ipcRenderer.send(events.POPULATE_BOOKMARKS, bookmarks);

    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
