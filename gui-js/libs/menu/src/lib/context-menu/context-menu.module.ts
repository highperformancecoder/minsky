import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { ColorPickerModule } from 'ngx-color-picker';
import { ContextMenuRoutingModule } from './context-menu-routing.module';
import { PenStylesComponent } from './pen-styles/pen-styles.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
  declarations: [PenStylesComponent],
  imports: [
    CommonModule,
    ContextMenuRoutingModule,
    ColorPickerModule,
    FormsModule,
    ReactiveFormsModule
  ],
})
export class ContextMenuModule {}
