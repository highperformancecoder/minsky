import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { ReactiveFormsModule } from '@angular/forms';

import { ColorPickerModule } from 'ngx-color-picker';
import { BackgroundColorComponent } from './background-color/background-color.component';
import { OptionsRoutingModule } from './options-routing.module';
import { PreferencesComponent } from './preferences/preferences.component';

@NgModule({
    imports: [
    CommonModule,
    ReactiveFormsModule,
    OptionsRoutingModule,
    ColorPickerModule,
    PreferencesComponent, BackgroundColorComponent,
],
})
export class OptionsModule {}
