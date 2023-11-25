import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { UiComponentsModule } from '@minsky/ui-components';
import { CreateVariableComponent } from './create-variable/create-variable.component';
import { InsertRoutingModule } from './insert-routing.module';

@NgModule({
  declarations: [CreateVariableComponent],
  imports: [
    CommonModule,
    InsertRoutingModule,
    UiComponentsModule,
  ],
})
export class InsertModule {}
