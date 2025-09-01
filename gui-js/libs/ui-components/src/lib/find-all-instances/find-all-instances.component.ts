import { Component } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { MatButtonModule } from '@angular/material/button';


@Component({
    selector: 'minsky-find-all-instances',
    templateUrl: './find-all-instances.component.html',
    styleUrls: ['./find-all-instances.component.scss'],
    standalone: true,
    imports: [MatButtonModule],
})
export class FindAllInstancesComponent {
  instances: string[];

  constructor(private electronService: ElectronService) {
    (async () => {
      await this.electronService.minsky.listAllInstances();
      this.instances = await this.electronService.minsky.variableInstanceList.names();
    })();
  }

  async goToInstance(i) {
    if (this.electronService.isElectron) {
      await this.electronService.minsky.variableInstanceList.gotoInstance(i);
      await this.electronService.minsky.canvas.requestRedraw();
    }
  }

  closeWindow() {this.electronService.closeWindow();}
}
