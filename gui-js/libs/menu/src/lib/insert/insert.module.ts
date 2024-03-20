import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CreateVariableComponent } from '@minsky/ui-components';
import { InsertRoutingModule } from './insert-routing.module';

@NgModule({
    imports: [
    CommonModule,
    InsertRoutingModule,
    CreateVariableComponent,
],
})
export class InsertModule {}
