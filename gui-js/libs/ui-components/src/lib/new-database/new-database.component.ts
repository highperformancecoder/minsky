import { ChangeDetectorRef, Component, OnInit, SimpleChanges } from '@angular/core';
import { FormsModule, } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { events} from '@minsky/shared';
import { MatAutocompleteModule } from '@angular/material/autocomplete';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';
import { NgIf, NgFor} from '@angular/common';
import { OpenDialogOptions, SaveDialogOptions } from 'electron';
import { CommonModule } from '@angular/common'; // Often useful for ngIf, ngFor
import JSON5 from 'json5';
import { ComboBoxComponent } from '../combo-box/combo-box.component';

@Component({
    selector: 'connect-database',
    templateUrl: './new-database.html',
    styleUrls: ['./new-database.scss'],
    standalone: true,
    imports: [
      FormsModule,
      ComboBoxComponent,
      CommonModule,
      MatAutocompleteModule,
      MatButtonModule,
      MatOptionModule,
      NgIf,
      NgFor,
    ],
})
export class NewDatabaseComponent implements OnInit {
  dbType="sqlite3";
  connection: string;
  table="";
  tables=[];
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private cdRef: ChangeDetectorRef
  ) {
  }

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
    });
  }

  setDbType(event) {
    const target = event.target as HTMLSelectElement;
    this.dbType=target.value;
  }

  setConnection(event) {
    const target = event.target as HTMLSelectElement;
    this.connection=target.value;    
  }

  // get list of tables
  async getTables() {
    this.electronService.minsky.databaseIngestor.db.connect(this.dbType,this.connection,"");
    this.tables=await this.electronService.minsky.databaseIngestor.db.tableNames();
  }
  
  setTable(event) {
    const target = event.target as HTMLSelectElement;
    this.table=target.value;    
  }

  setTableInput(event) {
    let input=document.getElementById("table") as HTMLInputElement;
    input.value=event?.option?.value;
  }
  
  async selectFile() {
    let options: OpenDialogOptions = {
      title: 'Select existing database',
      filters: [
        { extensions: ['sqlite'], name: 'SQLite' },
        { extensions: ['*'], name: 'All Files' },
      ],
    };
    //if (defaultPath) options['defaultPath'] = defaultPath;
    let filePath = await this.electronService.openFileDialog(options);
    if (filePath)
      this.connection=`db=${filePath}`;
    else {
      // if the user cancelled, then try to create a new database file
      options.title='Create new database';
      let filePath = await this.electronService.saveFileDialog(options);
      if (filePath)
        this.connection=`db=${filePath}`;
    }
    if (this.connection) {
      let connectionInput=document.getElementById("connection") as HTMLInputElement;
      connectionInput.hidden=false;
      connectionInput.value=this.connection;
      this.getTables();
    }
  }

  connect() {
    this.electronService.minsky.databaseIngestor.db.connect(this.dbType,this.connection,this.table);
    // TODO - set dropTable according to whether an existing table is selected, or a new given
    let dropTable=!(this.table in this.tables);
    this.electronService.invoke(events.IMPORT_CSV_TO_DB, {dropTable});
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
