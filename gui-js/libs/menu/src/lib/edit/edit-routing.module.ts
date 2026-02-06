import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { DimensionsComponent } from './dimensions/dimensions.component';
import {AuthorComponent } from './author/author';

const routes: Routes = [
  { path: 'author', component: AuthorComponent },
  { path: 'dimensions', component: DimensionsComponent },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule],
})
export class EditRoutingModule {}
