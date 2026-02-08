import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AuthorComponent } from './author/author';
import { DimensionsComponent } from './dimensions/dimensions.component';
import { EditRoutingModule } from './edit-routing.module';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
  imports: [CommonModule, EditRoutingModule, FormsModule, ReactiveFormsModule, AuthorComponent, DimensionsComponent],
})
export class EditModule {}
