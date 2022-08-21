import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { replaceBackSlash } from '@minsky/shared';
import { commandsMapping, events } from '@minsky/shared';

@Component({
  selector: 'minsky-edit-description',
  templateUrl: './edit-description.component.html',
  styleUrls: ['./edit-description.component.scss'],
})
export class EditDescriptionComponent implements OnInit {
  tooltip = '';
  detailedText = '';
  type = '';
  bookmark='false';

  editDescriptionForm: FormGroup;
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.bookmark = params['bookmark'];
      this.tooltip = params['tooltip'];
      this.detailedText = params['detailedText'];
      this.type = params['type'];
    });

    this.editDescriptionForm = new FormGroup({
      bookmark: new FormControl(this.bookmark),
      tooltip: new FormControl(this.tooltip),
      detailedText: new FormControl(this.detailedText),
    });
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.saveDescription({
        item: this.type,
        tooltip: this.editDescriptionForm.get('tooltip').value,
        detailedText: this.editDescriptionForm.get('detailedText').value,
        bookmark: this.editDescriptionForm.get('bookmark').value,
      });
    }
    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
