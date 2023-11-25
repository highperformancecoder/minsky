import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import {
  CliInputComponent,
  EditDescriptionComponent,
  EditGodleyCurrencyComponent,
  EditGodleyTitleComponent,
  EditGroupComponent,
  EditIntegralComponent,
  EditOperationComponent,
  EditUserFunctionComponent,
  EquationsComponent,
  FindAllInstancesComponent,
  GodleyWidgetViewComponent,
  ImportCsvComponent,
  PageNotFoundComponent,
  SummaryComponent,
  PlotWidgetOptionsComponent,
  PlotWidgetViewComponent,
  RavelViewComponent,
  RenameAllInstancesComponent,
  VariablePaneComponent,
  WiringComponent,     
  EditHandleDescriptionComponent,
  EditHandleDimensionComponent, 
  PickSlicesComponent,
  LockHandlesComponent
} from '@minsky/ui-components';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'wiring',
    pathMatch: 'full',
  },
  {
    path: 'wiring',
    component: WiringComponent,
  },
  {
    path: 'equations',
    component: EquationsComponent,
  },
  {
    path: 'itemTab/:tab',
    component: SummaryComponent,
  },
  {
    path: 'headless/menu',
    loadChildren: () => import('@minsky/menu').then((m) => m.MenuModule),
  },
  {
    path: 'headless/rename-all-instances',
    component: RenameAllInstancesComponent,
  },
  {
    path: 'headless/edit-operation',
    component: EditOperationComponent,
  },
  {
    path: 'headless/edit-userfunction',
    component: EditUserFunctionComponent,
  },
  {
    path: 'headless/edit-intop',
    component: EditIntegralComponent,
  },
  {
    path: 'headless/edit-group',
    component: EditGroupComponent,
  },
  {
    path: 'headless/edit-godley-title',
    component: EditGodleyTitleComponent,
  },
  {
    path: 'headless/edit-godley-currency',
    component: EditGodleyCurrencyComponent,
  },
  {
    path: 'headless/edit-description',
    component: EditDescriptionComponent,
  },
  {
    path: 'headless/edit-handle-description',
    component: EditHandleDescriptionComponent,
  },
  {
    path: 'headless/edit-handle-dimension',
    component: EditHandleDimensionComponent,
  },
  {
    path: 'headless/pick-slices',
    component: PickSlicesComponent,
  },
  {
    path: 'headless/lock-handles',
    component: LockHandlesComponent,
  },
  {
    path: 'headless/find-all-instances',
    component: FindAllInstancesComponent,
  },
  {
    path: 'headless/variable-pane',
    component: VariablePaneComponent,
  },
  {
    path: 'headless/plot-widget-view',
    component: PlotWidgetViewComponent,
  },
  {
    path: 'headless/godley-widget-view',
    component: GodleyWidgetViewComponent,
  },
  {
    path: 'headless/plot-widget-options',
    component: PlotWidgetOptionsComponent,
  },
  {
    path: 'headless/ravel-widget-view',
    component: RavelViewComponent,
  },
  {
    path: 'headless/terminal',
    component: CliInputComponent,
  },
  {
    path: 'headless/import-csv',
    component: ImportCsvComponent,
  },
  {
    path: '**',
    component: PageNotFoundComponent,
  },
];

@NgModule({
  imports: [
    RouterModule.forRoot(routes, {
      useHash: true
    }),
  ],
  exports: [RouterModule],
})
export class AppRoutingModule {}
