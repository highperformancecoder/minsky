import { Component } from '@angular/core';
import { AbstractControl, FormControl, Validators, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ThemePalette } from '@angular/material/core';
import { CommunicationService, ElectronService } from '@minsky/core';
import { MatButtonModule } from '@angular/material/button';
import { ColorPickerModule } from 'ngx-color-picker';
@Component({
    selector: 'minsky-background-color',
    templateUrl: './background-color.component.html',
    styleUrls: ['./background-color.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ColorPickerModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class BackgroundColorComponent {
  public disabled = false;
  public color: ThemePalette = 'primary';
  public touchUi = false;
  colorCtr = new FormControl('#c1c1c1', [Validators.required]);

  public options = [
    { value: true, label: 'True' },
    { value: false, label: 'False' },
  ];

  public listColors = ['primary', 'accent', 'warn'];

  constructor(
    private communicationService: CommunicationService,
    private electronService: ElectronService
  ) {}

  onClickOk() {
    this.communicationService.setBackgroundColor(this.colorCtr.value);

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
