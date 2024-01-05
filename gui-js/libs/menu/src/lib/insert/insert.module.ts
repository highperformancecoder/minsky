import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CreateVariableComponent } from './create-variable/create-variable.component';
import { InsertRoutingModule } from './insert-routing.module';

@NgModule({
    imports: [
    CommonModule,
    InsertRoutingModule,
    CreateVariableComponent,
],
})
export class InsertModule {}
