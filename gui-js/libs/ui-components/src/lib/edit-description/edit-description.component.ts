import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { events, Item, Wire } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'minsky-edit-description',
    templateUrl: './edit-description.component.html',
    styleUrls: ['./edit-description.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class EditDescriptionComponent implements OnInit {
  tooltip = '';
  detailedText = '';
  type = '';
  bookmark=false;

  editDescriptionForm: FormGroup;
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.bookmark = params['bookmark']==='true';
      this.tooltip = params['tooltip'];
      this.detailedText = params['detailedText'];
      this.type = params['type'];
    });

    this.editDescriptionForm = new FormGroup({
      bookmark: new FormControl(this.bookmark as boolean),
      tooltip: new FormControl(this.tooltip),
      detailedText: new FormControl(this.detailedText),
    });
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      var item: Item|Wire;
      switch (this.type) {
      case 'item':
        item=this.electronService.minsky.canvas.item;
        break;
      case 'wire':
        item=this.electronService.minsky.canvas.wire;
        break;
      }
      
      item.bookmark(this.editDescriptionForm.get('bookmark').value);
      item.tooltip(this.editDescriptionForm.get('tooltip').value);
      item.detailedText(this.editDescriptionForm.get('detailedText').value);
      item.adjustBookmark();
      item.updateBoundingBox();
      this.electronService.minsky.requestRedraw();
      this.electronService.send(events.UPDATE_BOOKMARK_LIST);
    }
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
