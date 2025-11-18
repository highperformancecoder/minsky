import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import { FormsModule, } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { Ravel} from '@minsky/shared';
import { MatAutocompleteModule } from '@angular/material/autocomplete';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';

@Component({
    selector: 'ravel-select-horizontal-dim',
    templateUrl: './ravel-select-horizontal-dim.html',
    styleUrls: ['./ravel-select-horizontal-dim.scss'],
    standalone: true,
    imports: [
    FormsModule,
    MatAutocompleteModule,
    MatButtonModule,
    MatOptionModule
],
})
export class RavelSelectHorizontalDimComponent implements OnInit  {
  dataCols=[];
  horizontalDimCols=new Set<string>;
  horizontalDimName="?";
  ravel: Ravel;
  
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private cdRef: ChangeDetectorRef
  ) {
    this.ravel=new Ravel(this.electronService.minsky.canvas.item);
  }

  async ngOnInit() {
    this.dataCols=await this.ravel.db.numericalColumnNames();
  }
  
  clickDim(event: Event) {
    const target = event.target as HTMLInputElement;
    if (target.checked)
      this.horizontalDimCols.add(target.name);
    else
      this.horizontalDimCols.delete(target.name);
  }

  setHorizontalNames() {
    this.ravel.db.setAxisNames([...this.horizontalDimCols],this.horizontalDimName);
    this.ravel.initRavelFromDb();
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
