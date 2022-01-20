import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { BackgroundColorComponent } from './background-color/background-color.component';
import { PreferencesComponent } from './preferences/preferences.component';

const routes: Routes = [
  { path: 'preferences', component: PreferencesComponent },
  { path: 'background-color', component: BackgroundColorComponent },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class OptionsRoutingModule {}
