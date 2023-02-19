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

  // keep track of scale and zoom factors separately, because using zoom exclusively creates rounding errors
  scaleBase = 1.1;
  scalePower = 1;
  scaleStep = 1 / 53;
  zoomFactor = 1;

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

  changeScale(e) {
    if(e.ctrlKey) {
      this.scalePower -= e.deltaY * this.scaleStep;

      this.zoomFactor = Math.pow(this.scaleBase, this.scalePower);
    }
  }
  
}
