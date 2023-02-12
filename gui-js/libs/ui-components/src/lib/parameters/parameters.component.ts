import { Component } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { ActivatedRoute } from '@angular/router';
import { switchMap } from 'rxjs/operators';

@Component({
  selector: 'minsky-parameters',
  templateUrl: './parameters.component.html',
  styleUrls: ['./parameters.component.scss'],
})
export class ParametersComponent {
  variables = [];

  tensorText = '<tensor>';

  type: string;

  constructor(private electronService: ElectronService, route: ActivatedRoute) {
    route.params.pipe(switchMap(p => {
      this.type = p['tab'];
      if(this.type === 'parameters') {
        return electronService.minsky.parameterTab.getDisplayVariables();
      } else {
        // populate required to make removal possible
        electronService.minsky.variableTab.populateItemVector();
        return electronService.minsky.variableTab.getDisplayVariables();
      }
    })).subscribe((v: any) => this.prepareVariables(v));
  }

  prepareVariables(variables) {
    this.variables = variables;
  }

  async onRemoveClick(variable) {
    const variableIndex = this.variables.indexOf(variable);
    this.electronService.minsky.variableTab.toggleVarDisplay(variableIndex + 1); // 1-based indexing

    this.variables.splice(variableIndex, 1);
  }
}
