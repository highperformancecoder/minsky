import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SharedModule } from '@minsky/shared';
import { CliInputComponent } from './cli-input/cli-input.component';
import { EditDescriptionComponent } from './edit-description/edit-description.component';
import { EditHandleDescriptionComponent } from './edit-handle-description/edit-handle-description.component';
import { EditHandleDimensionComponent } from './edit-handle-dimension/edit-handle-dimension.component';
import { EditGodleyCurrencyComponent } from './edit-godley-currency/edit-godley-currency.component';
import { EditGodleyTitleComponent } from './edit-godley-title/edit-godley-title.component';
import { EditGroupComponent } from './edit-group/edit-group.component';
import { EditIntegralComponent } from './edit-integral/edit-integral.component';
import { EditOperationComponent } from './edit-operation/edit-operation.component';
import { EditUserFunctionComponent } from './edit-user-function/edit-user-function.component';
import { EquationsComponent } from './equations/equations.component';
import { FindAllInstancesComponent } from './find-all-instances/find-all-instances.component';
import { GodleyWidgetViewComponent } from './godley-widget-view/godley-widget-view.component';
import { HeaderComponent } from './header/header.component';
import { ImportCsvComponent } from './import-csv/import-csv.component';
import { InputModalComponent } from './input-modal/input-modal.component';
import { PageNotFoundComponent } from './page-not-found/page-not-found.component';
import { ParametersComponent } from './parameters/parameters.component';
import { PlotWidgetOptionsComponent } from './plot-widget-options/plot-widget-options.component';
import { PlotWidgetViewComponent } from './plot-widget-view/plot-widget-view.component';
import { RenameAllInstancesComponent } from './rename-all-instances/rename-all-instances.component';
import { ToolbarComponent } from './toolbar/toolbar.component';
import { VariablesComponent } from './variables/variables.component';
import { AvailableOperationsComponent } from './wiring/available-operations/available-operations.component';
import { VariableComponent } from './wiring/variable/variable.component';
import { WiringComponent } from './wiring/wiring.component';
import { PickSlicesComponent } from './pick-slices/pick-slices.component';
import { LockHandlesComponent } from './lock-handles/lock-handles.component';

@NgModule({
  declarations: [
    HeaderComponent,
    ToolbarComponent,
    PageNotFoundComponent,
    WiringComponent,
    EquationsComponent,
    ParametersComponent,
    VariablesComponent,
    CliInputComponent,
    AvailableOperationsComponent,
    VariableComponent,
    RenameAllInstancesComponent,
    InputModalComponent,
    EditDescriptionComponent,
    EditHandleDescriptionComponent,
    EditHandleDimensionComponent,
    PickSlicesComponent,
    LockHandlesComponent,
    EditGodleyTitleComponent,
    EditGodleyCurrencyComponent,
    FindAllInstancesComponent,
    EditOperationComponent,
    EditIntegralComponent,
    EditGroupComponent,
    EditUserFunctionComponent,
    PlotWidgetViewComponent,
    GodleyWidgetViewComponent,
    PlotWidgetOptionsComponent,
    ImportCsvComponent,
  ],
  imports: [CommonModule, SharedModule],
  exports: [
    PageNotFoundComponent,
    HeaderComponent,
    ToolbarComponent,
    WiringComponent,
    EquationsComponent,
    ParametersComponent,
    VariablesComponent,
    CliInputComponent,
    RenameAllInstancesComponent,
    InputModalComponent,
    EditDescriptionComponent,
    EditHandleDescriptionComponent,
    EditHandleDimensionComponent,
    PickSlicesComponent,
    LockHandlesComponent,
    EditGodleyTitleComponent,
    EditGodleyCurrencyComponent,
    FindAllInstancesComponent,
    EditOperationComponent,
    EditIntegralComponent,
    EditGroupComponent,
    EditUserFunctionComponent,
    PlotWidgetViewComponent,
    GodleyWidgetViewComponent,
    PlotWidgetOptionsComponent,
    ImportCsvComponent,
  ],
})
export class UiComponentsModule {}
