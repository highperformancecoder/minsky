import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SharedModule } from '@minsky/shared';
import { SimulationRoutingModule } from './simulation-routing.module';
import { SimulationParametersComponent } from './simulation/simulation-parameters.component';

@NgModule({
  declarations: [SimulationParametersComponent],
  imports: [CommonModule, SimulationRoutingModule, SharedModule],
})
export class SimulationModule {}
