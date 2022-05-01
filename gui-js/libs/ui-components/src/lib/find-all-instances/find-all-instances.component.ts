import { Component } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';

@Component({
  selector: 'minsky-find-all-instances',
  templateUrl: './find-all-instances.component.html',
  styleUrls: ['./find-all-instances.component.scss'],
})
export class FindAllInstancesComponent {
  instances: string[];

  constructor(private electronService: ElectronService) {
    (async () => {
      await electronService.sendMinskyCommandAndRender({
        command: commandsMapping.LIST_ALL_INSTANCES,
      });

      this.instances = (await this.electronService.sendMinskyCommandAndRender({
        command: commandsMapping.VARIABLE_INSTANCE_LIST_NAMES,
      })) as string[];
    })();
  }

  async goToInstance(i) {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.VARIABLE_INSTANCE_LIST_GOTO_INSTANCE} ${i}`,
      });
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.REQUEST_REDRAW_SUBCOMMAND} `,
      });
    }
  }

  closeWindow() {
    this.electronService.remote.getCurrentWindow().close();
  }
}
