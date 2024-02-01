import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { PubTab } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'minsky-new-pub-tab',
    templateUrl: './new-pub-tab.component.html',
    styleUrls: ['./new-pub-tab.component.scss', '../generic-form.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class NewPubTabComponent implements OnInit {
  pubTab: PubTab;
  type: string;

  editDescriptionForm: FormGroup;
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  async ngOnInit() {
    this.route.queryParams.subscribe((params) => {
      this.pubTab=new PubTab(params['command']);
      this.type=params['type'];
    });
    this.editDescriptionForm = new FormGroup({
      description: new FormControl(''),
    });
    if (this.type==='rename')
      this.editDescriptionForm.get('description').setValue(await this.pubTab.name());
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      let name=this.editDescriptionForm.get('description').value;
      switch (this.type) {
        case 'new':
        await this.electronService.minsky.addNewPublicationTab(name);
        break;
        case 'rename':
        await this.pubTab.name(name);
        break;
      }
    }
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
