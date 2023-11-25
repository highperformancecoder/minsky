import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { ReactiveFormsModule } from '@angular/forms';
import { MaterialModule } from '@minsky/ui-components';
import { ColorPickerModule } from 'ngx-color-picker';
import { BackgroundColorComponent } from './background-color/background-color.component';
import { OptionsRoutingModule } from './options-routing.module';
import { PreferencesComponent } from './preferences/preferences.component';

@NgModule({
  declarations: [PreferencesComponent, BackgroundColorComponent],
  imports: [
    CommonModule,
    ReactiveFormsModule,
    OptionsRoutingModule,
    MaterialModule,
    ColorPickerModule,
  ],
})
export class OptionsModule {}
