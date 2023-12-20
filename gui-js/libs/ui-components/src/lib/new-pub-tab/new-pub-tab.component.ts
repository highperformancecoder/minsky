import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';

@Component({
  selector: 'minsky-new-pub-tab',
  templateUrl: './new-pub-tab.component.html',
  styleUrls: ['./new-pub-tab.component.scss', '../generic-form.scss'],
})
export class NewPubTabComponent implements OnInit {
  command: string;
  handleIndex: number;

  editDescriptionForm: FormGroup;
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.command=params['command'];
      this.handleIndex = +params['handleIndex'];
      this.editDescriptionForm = new FormGroup({
        description: new FormControl(params['description']),
      });
    });
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.minsky.addNewPublicationTab(this.editDescriptionForm.get('description').value);
    }
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
