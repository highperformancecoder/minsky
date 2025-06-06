import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import { FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { events, Item, Wire } from '@minsky/shared';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'connect-database',
    templateUrl: './connect-database.html',
    styleUrls: ['./connect-database.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class ConnectDatabaseComponent implements OnInit {

  connectDatabaseForm: FormGroup;
  dbType="sqlite3";
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private cdRef: ChangeDetectorRef
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
    });

    this.connectDatabaseForm = new FormGroup({
      dbType: new FormControl(),
      connectionString: new FormControl(),
    });
  }

  setDbType(event) {
    const target = event.target as HTMLSelectElement;
    this.dbType=target.value;
  }

  async selectFile() {
    let options: OpenDialogOptions = {
      filters: [
        { extensions: ['sqlite'], name: 'CSV' },
        { extensions: ['*'], name: 'All Files' },
      ],
      properties: ['openFile'],
    };
    //if (defaultPath) options['defaultPath'] = defaultPath;
    this.filePath = await this.electronService.openFileDialog(options);
  }
  
  async connect() {
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
