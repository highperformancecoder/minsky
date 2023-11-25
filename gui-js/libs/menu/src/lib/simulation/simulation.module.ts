import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SimulationRoutingModule } from './simulation-routing.module';
import { SimulationParametersComponent } from './simulation/simulation-parameters.component';
import { UiComponentsModule } from '@minsky/ui-components';

@NgModule({
  declarations: [SimulationParametersComponent],
  imports: [CommonModule, SimulationRoutingModule, UiComponentsModule],
})
export class SimulationModule {}
