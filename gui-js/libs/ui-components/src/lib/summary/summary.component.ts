import { Component, OnInit } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { ActivatedRoute } from '@angular/router';
import { switchMap } from 'rxjs/operators';
import { ScaleHandler } from '../scale-handler/scale-handler.class';

@Component({
  selector: 'minsky-parameters',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.scss'],
})
export class SummaryComponent implements OnInit {
  allVariables;
  globalVariables;
  groups={};
  godleys={};

  labels={allVariables: 'All Variables', globalVariables: 'Global Variables', groups: 'Local Variables', godleys: 'Godley Variables'};
  numVars={allVariables: 0, globalVariables: 0, groups: 0, godleys: 0};
    
  scale = new ScaleHandler();

  constructor(private electronService: ElectronService, route: ActivatedRoute) {}

  append(o: Object, key: string, v) {
    if (o[key])
      o[key].push(v);
    else
      o[key]=[v];
  }
  
  async ngOnInit() {
    let variables=await this.electronService.minsky.variableValues.summarise();
    this.allVariables={};
    this.globalVariables={};
    for (let v in variables) {
      let type=variables[v]["type"];
      this.append(this.allVariables, type, variables[v]);
      if (variables[v]["scope"]===":")
        this.append(this.globalVariables, type, variables[v]);
      else
        this.append(this.groups, variables[v]["scope"], variables[v]);
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
    for (let name in this.groups)
    {
      this.groups[name].sort((x,y)=>{return x.type<y.type || x.type===y.type && x.name<y.name;});
      this.numVars[name]=this.groups[name].length;
      this.numVars.groups+=this.groups[name].length;
    }

    // now process Godley table variables
    variables=await this.electronService.minsky.model.summariseGodleys();
    for (let v in variables) {
      this.append(this.godleys, variables[v]["godley"], variables[v]);
    }
    for (let name in this.godleys)
    {
      this.godleys[name].sort((x,y)=>{return x.type<y.type || x.type===y.type && x.name<y.name;});
      this.numVars[name]=this.godleys[name].length;
      this.numVars.godleys+=this.godleys[name].length;
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
    for (let i=0; i<nestedElements.length; ++i) 
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
