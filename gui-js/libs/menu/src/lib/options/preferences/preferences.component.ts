import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { events } from '@minsky/shared';

@Component({
  selector: 'minsky-preferences',
  templateUrl: './preferences.component.html',
  styleUrls: ['./preferences.component.scss'],
})
export class PreferencesComponent implements OnInit {
  form: FormGroup;
  availableFonts: string[] = [];

  constructor(private electronService: ElectronService) {
    this.form = new FormGroup({
      godleyTableShowValues: new FormControl(null),
      godleyTableOutputStyle: new FormControl(null),
      enableMultipleEquityColumns: new FormControl(null),
      numberOfRecentFilesToDisplay: new FormControl(null),
      wrapLongEquationsInLatexExport: new FormControl(null),
      // focusFollowsMouse: new FormControl(null),
      font: new FormControl(null),
    });
  }
  async ngOnInit() {
    if (this.electronService.isElectron) {
      const preferences = await this.electronService.invoke(
        events.GET_PREFERENCES
      );
      this.form.patchValue(preferences);
      this.availableFonts = await this.electronService.minsky.listFonts();
    }
  }

  async updatePreferences() {
    if (this.electronService.isElectron) {
      const preferences = this.form.value;
      this.electronService.log(this.form.value);
      await this.electronService.invoke(
        events.UPDATE_PREFERENCES,
        preferences
      );
    }
  }
  
  async handleSubmit() {
    await this.updatePreferences();
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
