import { Component, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { GodleyIcon } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-edit-godley-title',
  templateUrl: './edit-godley-title.component.html',
  styleUrls: ['./edit-godley-title.component.scss'],
})
export class EditGodleyTitleComponent implements OnDestroy {
  title: string;
  itemId: string;
  
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.subscribe((params) => {
      this.title = params.title;
      this.itemId = params.itemId;
    });
  }

  async handleEditTitle(newTitle: string) {
    if (this.electronService.isElectron) {
        var godley=new GodleyIcon(
          this.itemId?
            this.electronService.minsky.namedItems.elem(this.itemId).second:
            this.electronService.minsky.canvas.item
        )
      godley.table.title(newTitle);
      this.electronService.minsky.canvas.requestRedraw();
      this.windowUtilityService.closeCurrentWindowIfNotMain();
    }
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
