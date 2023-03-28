import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { ActivatedRoute } from '@angular/router';
import { switchMap } from 'rxjs/operators';
import { ScaleHandler } from '../scale-handler/scale-handler.class';
import { uniqBy, sortBy } from 'lodash-es';
import * as JSON5 from 'json5';

@Component({
  selector: 'minsky-parameters',
  templateUrl: './parameters.component.html',
  styleUrls: ['./parameters.component.scss'],
})

export class ParametersComponent implements OnInit {
  variables;

  tensorText = '<tensor>';

  type: string;

  scale = new ScaleHandler();

  constructor(private electronService: ElectronService, route: ActivatedRoute,
    private cdr: ChangeDetectorRef) {
//    setTimeout(async ()=>{
//      switch (route.params['tab']) {
//      case 'parameters':
//        this.prepareVariables(await electronService.minsky.parameterTab.getDisplayVariables());
//        break;
//      case 'variables':
//        this.prepareVariables(await electronService.minsky.variableTab.getDisplayVariables());
//        break;
//    };
    
    route.params.pipe(switchMap(p => {
      this.type = p['tab'];
      if(this.type === 'parameters') {
        return electronService.minsky.parameterTab.getDisplayVariables();
      } else {
        return electronService.minsky.variableTab.getDisplayVariables();
      }
    })).subscribe(async (v: any) => this.prepareVariables(await v));
//    }, 1);
  }

  ngOnInit() {
    this.electronService.log(`ngOnInit: ${JSON5.stringify(this.variables)}`);
  }
  
  prepareVariables(variables: any[]) {
    // why tf is this uniqBy necessary?! sort duplicates rows..? js' own .sort function does the same thing
    this.variables = uniqBy(sortBy(variables, ['type', 'name']), v => v.name);
    this.electronService.log(`ParametersComponent.variables=${JSON5.stringify(this.variables)}`);
    this.cdr.detectChanges();
  }

  changeScale(e) {
    if(e.ctrlKey) {
      this.scale.changeScale(e.deltaY);
    }
  }

  truncateValue(value: string) {
    if(isNaN(+value)) return value;
    const stringDecimals = String(+value - Math.floor(+value));
    if(stringDecimals.length > 6) return (+value).toFixed(4);
    return String(value);
  }
  
}
