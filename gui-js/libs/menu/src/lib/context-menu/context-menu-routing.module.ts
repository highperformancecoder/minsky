import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { PenStylesComponent } from './pen-styles/pen-styles.component';

const routes: Routes = [{ path: 'pen-styles', component: PenStylesComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class ContextMenuRoutingModule {}
