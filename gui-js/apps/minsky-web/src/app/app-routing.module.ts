import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import {
  ConnectDatabaseComponent,
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
  NewDatabaseComponent,
  NewPubTabComponent,
  PageNotFoundComponent,
  SummaryComponent,
  PlotWidgetOptionsComponent,
  PlotWidgetViewComponent,
  RavelSelectHorizontalDimComponent,
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
    path: 'headless/connect-database',
    component: ConnectDatabaseComponent,
  },
  {
    path: 'headless/edit-description',
    component: EditDescriptionComponent,
  },
  {
    path: 'headless/edit-godley-currency',
    component: EditGodleyCurrencyComponent,
  },
  {
    path: 'headless/edit-godley-title',
    component: EditGodleyTitleComponent,
  },
  {
    path: 'headless/edit-group',
    component: EditGroupComponent,
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
    path: 'headless/edit-intop',
    component: EditIntegralComponent,
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
    path: 'headless/find-all-instances',
    component: FindAllInstancesComponent,
  },
  {
    path: 'headless/godley-widget-view',
    component: GodleyWidgetViewComponent,
  },
  {
    path: 'headless/import-csv',
    component: ImportCsvComponent,
  },
  {
    path: 'headless/lock-handles',
    component: LockHandlesComponent,
  },
  {
    path: 'headless/new-database',
    component: NewDatabaseComponent,
  },
  {
    path: 'headless/new-pub-tab',
    component: NewPubTabComponent,
  },
  {
    path: 'headless/pick-slices',
    component: PickSlicesComponent,
  },
  {
    path: 'headless/plot-widget-view',
    component: PlotWidgetViewComponent,
  },
  {
    path: 'headless/plot-widget-options',
    component: PlotWidgetOptionsComponent,
  },
  {
    path: 'headless/ravel-select-horizontal-dim',
    component: RavelSelectHorizontalDimComponent,
  },
  {
    path: 'headless/ravel-widget-view',
    component: RavelViewComponent,
  },
  {
    path: 'headless/rename-all-instances',
    component: RenameAllInstancesComponent,
  },
  {
    path: 'headless/terminal',
    component: CliInputComponent,
  },
  {
    path: 'headless/variable-pane',
    component: VariablePaneComponent,
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
