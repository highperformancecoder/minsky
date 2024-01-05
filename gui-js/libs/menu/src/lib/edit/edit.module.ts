import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { DimensionsComponent } from './dimensions/dimensions.component';
import { EditRoutingModule } from './edit-routing.module';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
    imports: [CommonModule, EditRoutingModule, FormsModule, ReactiveFormsModule, DimensionsComponent],
})
export class EditModule {}
