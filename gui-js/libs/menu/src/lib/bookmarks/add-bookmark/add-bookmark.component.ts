import { Component } from '@angular/core';
import { FormControl, Validators } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { events, } from '@minsky/shared';

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
    let minsky=this.electronService.minsky;
    minsky.model.addBookmark(this.bookmarkName.value);
    this.electronService.ipcRenderer.send(events.POPULATE_BOOKMARKS, await minsky.model.bookmarkList());

    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
