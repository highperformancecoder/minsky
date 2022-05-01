import { NgModule } from '@angular/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialModule } from './material/material.module';

@NgModule({
  imports: [MaterialModule, FormsModule, ReactiveFormsModule],
  declarations: [],
  exports: [MaterialModule, FormsModule, ReactiveFormsModule],
})
export class SharedModule {}
