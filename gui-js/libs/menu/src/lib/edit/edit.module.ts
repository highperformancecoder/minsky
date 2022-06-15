import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SharedModule } from '@minsky/shared';
import { DimensionsComponent } from './dimensions/dimensions.component';
import { EditRoutingModule } from './edit-routing.module';

@NgModule({
  declarations: [DimensionsComponent],
  imports: [CommonModule, EditRoutingModule, SharedModule],
})
export class EditModule {}
