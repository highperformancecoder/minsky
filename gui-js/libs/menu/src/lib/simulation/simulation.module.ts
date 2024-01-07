import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SimulationRoutingModule } from './simulation-routing.module';
import { SimulationParametersComponent } from './simulation/simulation-parameters.component';


@NgModule({
    imports: [CommonModule, SimulationRoutingModule, SimulationParametersComponent],
})
export class SimulationModule {}
