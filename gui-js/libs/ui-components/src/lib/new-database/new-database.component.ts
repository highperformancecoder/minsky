import { ChangeDetectorRef, Component, SimpleChanges } from '@angular/core';
import { FormsModule, } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { events, Functions} from '@minsky/shared';
import { MatAutocompleteModule } from '@angular/material/autocomplete';
import { MatButtonModule } from '@angular/material/button';
import { MatOptionModule } from '@angular/material/core';
import { OpenDialogOptions, SaveDialogOptions } from 'electron';
import { CommonModule } from '@angular/common'; // Often useful for ngIf, ngFor

@Component({
    selector: 'new-database',
    templateUrl: './new-database.html',
    styleUrls: ['./new-database.scss'],
    standalone: true,
    imports: [
      FormsModule,
      CommonModule,
      MatAutocompleteModule,
      MatButtonModule,
      MatOptionModule,
    ],
})
export class NewDatabaseComponent {
  dbType: string='sqlite3';
  connection: string='';
  table: string='';
  tables: string[]=[];
  backends: string[]=[];
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private cdRef: ChangeDetectorRef
  ) {
  }

  async ngOnInit() {
    this.backends=await this.electronService.minsky.databaseIngestor.db.backends();
  }

  setDbType(event) {
    const target = event.target as HTMLSelectElement;
    this.dbType=target.value;
  }

  setConnection(event: Event) {
    const target = event.target as HTMLSelectElement;
    this.connection=target.value;    
  }

  // get list of tables
  async getTables() {
    this.electronService.minsky.databaseIngestor.db.connect(this.dbType,this.connection,"");
    this.tables=await this.electronService.minsky.databaseIngestor.db.tableNames();
  }
  
  setTable(event: Event) {
    const target = event.target as HTMLSelectElement;
    this.table=target.value;    
  }

  setTableInput(event) {
    this.table=event?.option?.value;
  }
  
  async selectFile() {
    let options: OpenDialogOptions = {
      title: 'Select existing database',
      defaultPath: ':models',
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
    if (this.connection) 
      await this.getTables();
  }

  connect() {
    // on Windows, the odbc driver will prompt for a DSN, so allow for empty connection data
    if (
      (!Functions.isWindows() || this.dbType!='odbc') &&
        (!this.connection || !this.table)
    ) {
      this.electronService.showMessageBoxSync({message: "Connection string or table not present"});
      return;
    }      
     this.electronService.minsky.databaseIngestor.db.connect(this.dbType,this.connection,this.table);
    let dropTable=!this.tables.includes(this.table);
    this.electronService.invoke(events.IMPORT_CSV_TO_DB, {dropTable});
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
