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

  editRow=null;  ///< row of the current cell being edited
  editCol='';   ///< column of the current cell being edited
  editCellContents=''; ///< contents of cell being edited
  
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
    {
      nestedElements[i].addEventListener("click",()=>this.toggleCaret({target:nestedElements[i]}));
      this.electronService.log(`${nestedElements[i].classList}`);
    }
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

  editing(variable, member: string): boolean {
    return variable===this.editRow && member===this.editCol; 
  }
  edit(event, variable, member: string) {
    event.stopImmediatePropagation();
    this.editRow=variable;
    this.editCol=member;
    switch (member)
    {
      case 'definition':
      this.editCellContents=variable.udfDefinition;
      break;
      default:
      this.editCellContents=variable[member];
      break;
    }
  }

  finishEditing() {
    this.editRow[this.editCol]=this.editCellContents;
    switch (this.editCol)
    {
      case 'init':
      this.electronService.minsky.variableValues.elem(this.editRow.valueId).second.init(this.editCellContents);
      break;
      case 'units':
      this.electronService.minsky.variableValues.elem(this.editRow.valueId).second.setUnits(this.editCellContents);
      break;
      case 'name':
      this.electronService.minsky.model.renameAllInstances(this.editRow.valueId, this.editCellContents);
      this.ngOnInit(); // force rereading of item valueIds.
      break;
      case 'definition':
      this.electronService.minsky.setDefinition(this.editRow.valueId, this.editCellContents);
      break;
    }
    this.editRow=null;
  }
  
}
