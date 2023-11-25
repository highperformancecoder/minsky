import { CommonModule } from '@angular/common';
import { HttpClientModule } from '@angular/common/http';
import { NgModule } from '@angular/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialModule } from '@minsky/ui-components';
import { DialogComponent } from './component/dialog/dialog.component';
@NgModule({
  imports: [
    CommonModule,
    HttpClientModule,
    MaterialModule,
    FormsModule,
    ReactiveFormsModule,
  ],
  declarations: [DialogComponent],
  exports: [DialogComponent],
})
export class CoreModule {}
