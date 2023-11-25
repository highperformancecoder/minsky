import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AboutComponent } from './about/about.component';
import { FileRoutingModule } from './file-routing.module';
import { LogSimulationComponent } from './log-simulation/log-simulation.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
  declarations: [AboutComponent, LogSimulationComponent],
  imports: [CommonModule, FileRoutingModule, FormsModule, ReactiveFormsModule],
})
export class FileModule {}
