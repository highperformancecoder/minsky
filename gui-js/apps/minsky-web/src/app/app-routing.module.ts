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
  ParametersComponent,
  PlotWidgetOptionsComponent,
  PlotWidgetViewComponent,
  RenameAllInstancesComponent,
  VariablesComponent,
  WiringComponent,
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
    path: 'parameters',
    component: ParametersComponent,
  },
  {
    path: 'variables',
    component: VariablesComponent,
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
    path: 'headless/find-all-instances',
    component: FindAllInstancesComponent,
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
      useHash: true,
      relativeLinkResolution: 'legacy',
    }),
  ],
  exports: [RouterModule],
})
export class AppRoutingModule {}
