import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SharedModule } from '@minsky/shared';
import { AboutComponent } from './about/about.component';
import { FileRoutingModule } from './file-routing.module';
import { LogSimulationComponent } from './log-simulation/log-simulation.component';

@NgModule({
  declarations: [AboutComponent, LogSimulationComponent],
  imports: [CommonModule, FileRoutingModule, SharedModule],
})
export class FileModule {}
