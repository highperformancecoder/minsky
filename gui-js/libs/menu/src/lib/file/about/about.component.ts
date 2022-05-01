import { Component, OnInit, VERSION } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';

@Component({
  selector: 'minsky-about',
  templateUrl: './about.component.html',
  styleUrls: ['./about.component.scss'],
})
export class AboutComponent implements OnInit {
  angularVersion = VERSION.full;
  ecolabVersion: string;
  minskyVersion: string;
  ravelVersion: string;

  constructor(private electronService: ElectronService) {}

  ngOnInit(): void {
    (async () => {
      const _ecolabVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.ECOLAB_VERSION,
        }
      )) as string;

      this.ecolabVersion = this.normalizeVersion(_ecolabVersion);

      const _ravelVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.RAVEL_VERSION,
        }
      )) as string;

      this.ravelVersion = this.normalizeVersion(_ravelVersion);

      const _minskyVersion = (await this.electronService.sendMinskyCommandAndRender(
        {
          command: commandsMapping.MINSKY_VERSION,
        }
      )) as string;

      this.minskyVersion = this.normalizeVersion(_minskyVersion);
    })();
  }

  private normalizeVersion(version: string) {
    const normalizedVersion =
      version === 'unavailable' ||
      JSON.stringify(version) === JSON.stringify({})
        ? ''
        : version;

    return normalizedVersion;
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
