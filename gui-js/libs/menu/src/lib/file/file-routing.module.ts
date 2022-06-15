import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { AboutComponent } from './about/about.component';
import { LogSimulationComponent } from './log-simulation/log-simulation.component';

const routes: Routes = [
  { path: 'about', component: AboutComponent },
  { path: 'log-simulation', component: LogSimulationComponent },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class FileRoutingModule {}
