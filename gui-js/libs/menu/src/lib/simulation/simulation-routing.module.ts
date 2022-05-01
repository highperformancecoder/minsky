import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SimulationParametersComponent } from './simulation/simulation-parameters.component';

const routes: Routes = [
  { path: 'simulation-parameters', component: SimulationParametersComponent },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class SimulationRoutingModule {}
