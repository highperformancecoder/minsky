import { Component, OnInit, VERSION } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { version } from '@minsky/shared';

@Component({
  selector: 'minsky-about',
  templateUrl: './about.component.html',
  styleUrls: ['./about.component.scss'],
})
export class AboutComponent implements OnInit {
  angularVersion = VERSION.full;
  ecolabVersion: string;
  minskyVersion: string;
  frontEndVersion: string;
  ravelVersion: string;

  constructor(private electronService: ElectronService){}

  ngOnInit(): void {
    (async () => {
      let minsky=this.electronService.minsky;
      this.ecolabVersion = await minsky.ecolabVersion();
      this.ravelVersion = await minsky.ravelVersion();
      this.minskyVersion = await minsky.minskyVersion();
      this.frontEndVersion=version;
    })();
  }

  async closeWindow() {this.electronService.closeWindow();}
}
