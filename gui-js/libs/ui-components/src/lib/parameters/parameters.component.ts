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

  constructor(electronService: ElectronService, route: ActivatedRoute) {
    route.params.pipe(switchMap(p => {
      this.type = p['tab'];
      if(this.type === 'parameters') {
        return electronService.minsky.parameterTab.getDisplayVariables();
      } else {
        return electronService.minsky.variableTab.getDisplayVariables();
      }
    })).subscribe((v: any) => this.prepareVariables(v));
  }

  prepareVariables(variables) {
    this.variables = variables;
  }
}
