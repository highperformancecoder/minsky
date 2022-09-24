import { Component, OnInit, VERSION } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { commandsMapping, version } from '@minsky/shared';

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

  constructor(private electronService: ElectronService) {}

  ngOnInit(): void {
    (async () => {
      this.ecolabVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.ECOLAB_VERSION,
        }
      )) as string;

      this.ravelVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.RAVEL_VERSION,
        }
      )) as string;


      this.minskyVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.MINSKY_VERSION,
        }
      )) as string;

      this.frontEndVersion=version;
    })();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
