import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { CreateVariableComponent } from './create-variable/create-variable.component';

const routes: Routes = [
  { path: 'create-variable', component: CreateVariableComponent },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class InsertRoutingModule {}
