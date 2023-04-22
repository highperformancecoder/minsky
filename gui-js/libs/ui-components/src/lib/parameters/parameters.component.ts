import { Component, OnInit } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { ActivatedRoute } from '@angular/router';
import { switchMap } from 'rxjs/operators';
import { ScaleHandler } from '../scale-handler/scale-handler.class';

@Component({
  selector: 'minsky-parameters',
  templateUrl: './parameters.component.html',
  styleUrls: ['./parameters.component.scss'],
})
export class ParametersComponent implements OnInit {
  allVariables;
  globalVariables;
  groups={};
  godleys={};

  labels={allVariables: 'All Variables', globalVariables: 'Global Variables', groups: 'Groups', godleys: 'Godleys'};
  numVars={allVariables: 0, globalVariables: 0, groups: 0, godleys: 0};
    
  scale = new ScaleHandler();

  constructor(private electronService: ElectronService, route: ActivatedRoute) {}

  async ngOnInit() {
    let variables=await this.electronService.minsky.variableValues.summarise();
    this.allVariables={};
    this.globalVariables={};
    for (let v in variables) {
      let type=variables[v]["type"];
      if (this.allVariables[type])
        this.allVariables[type].push(variables[v]);
      else
        this.allVariables[type]=[variables[v]];
      if (variables[v]["scope"]===":")
      {
        if (this.globalVariables[type])
          this.globalVariables[type].push(variables[v]);
        else
          this.globalVariables[type]=[variables[v]];
      }
    }

   for (let type in this.allVariables)
    {
      this.allVariables[type].sort((x,y)=>{return x.name<y.name;});
      this.numVars.allVariables+=this.allVariables[type].length;
    }
    for (let type in this.globalVariables)
    {
      this.globalVariables[type].sort((x,y)=>{return x.name<y.name;});
      this.numVars.globalVariables+=this.globalVariables[type].length;
    }
  }

  types(category: string): string[] {
    if (!this[category]) return [];
    return Object.keys(this[category]).sort();
  }
  
  toggleCaret(event) {
    event.target.classList.toggle("caret-down");
    let nestedElements=event.target.childNodes;
    for (let i in nestedElements) 
      nestedElements[i].classList?.toggle("active");
    nestedElements=event.target.querySelectorAll(".caret");
    // why do we need to add this event listener? Why isn't the (click)= annotation enough?
    for (let i in nestedElements) 
      nestedElements[i].addEventListener("click",()=>this.toggleCaret({target:nestedElements[i]}));
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
