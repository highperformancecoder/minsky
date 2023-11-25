import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { DimensionsComponent } from './dimensions/dimensions.component';
import { EditRoutingModule } from './edit-routing.module';
import { MaterialModule } from '@minsky/ui-components';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
  declarations: [DimensionsComponent],
  imports: [CommonModule, EditRoutingModule, MaterialModule, FormsModule, ReactiveFormsModule],
})
export class EditModule {}
