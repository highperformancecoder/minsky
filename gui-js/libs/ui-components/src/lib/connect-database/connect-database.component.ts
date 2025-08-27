import { ChangeDetectorRef, Component, SimpleChanges } from '@angular/core';
import { FormsModule, } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { Ravel} from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';
import { OpenDialogOptions } from 'electron';
import { CommonModule } from '@angular/common'; // Often useful for ngIf, ngFor

@Component({
    selector: 'connect-database',
    templateUrl: './connect-database.html',
    styleUrls: ['./connect-database.scss'],
    standalone: true,
    imports: [
        FormsModule,
        CommonModule,
        MatButtonModule,
    ],
})
export class ConnectDatabaseComponent {
  dbType: string="sqlite3";
  connection: string;
  table: string="";
  tables: string[]=[];
  backends: string[]=[];
  ravel: Ravel;
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private cdRef: ChangeDetectorRef
  ) {
    this.ravel=new Ravel(this.electronService.minsky.canvas.item);
  }

  async ngOnInit() {
    this.backends=await this.ravel.db.backends();
  }

  setDbType(event) {
    const target = event.target as HTMLSelectElement;
    this.dbType=target.value;
  }

  // get list of tables
  async getTables() {
    this.ravel.db.connect(this.dbType,this.connection,"");
    this.tables=await this.ravel.db.tableNames();
  }
  
  setTable(event: Event) {
    const target = event.target as HTMLSelectElement;
    this.table=target.value;    
  }

  setConnection(event: Event) {
    const target = event.target as HTMLSelectElement;
    this.connection=target.value;    
  }
  
  async selectFile() {
    let options: OpenDialogOptions = {
      defaultPath: ':models',
      filters: [
        { extensions: ['sqlite'], name: 'CSV' },
        { extensions: ['*'], name: 'All Files' },
      ],
      properties: ['openFile'],
    };
    //if (defaultPath) options['defaultPath'] = defaultPath;
    let filePath = await this.electronService.openFileDialog(options);
    if (typeof filePath==='string')
      this.connection=`db=${filePath}`;
    else
      this.connection=`db=${filePath[0]}`;
    await this.getTables();
  }

  connect() {
    this.ravel.db.connect(this.dbType,this.connection,this.table);
    this.ravel.initRavelFromDb();
    this.electronService.minsky.canvas.requestRedraw();
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
