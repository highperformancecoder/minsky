import { Component } from '@angular/core';
import { CommunicationService, ElectronService } from '@minsky/core';
import { events } from '@minsky/shared';
import { MatMenuModule } from '@angular/material/menu';

@Component({
    selector: 'minsky-variable',
    templateUrl: './variable.component.html',
    styleUrls: ['./variable.component.scss'],
    standalone: true,
    imports: [MatMenuModule],
})
export class VariableComponent {
  constructor(
    public communicationService: CommunicationService,
    private electronService: ElectronService
  ) {}
  createVariable(type: string) {
    if (this.electronService.isElectron) {
      let url = '';
      switch (type) {
        case 'flow':
          url = `#/headless/menu/insert/create-variable?type=flow`;
          break;

        case 'constant':
          url = `#/headless/menu/insert/create-variable?type=constant`;
          break;

        case 'parameter':
          url = `#/headless/menu/insert/create-variable?type=parameter`;
          break;

        default:
          break;
      }
      this.electronService.send(events.CREATE_MENU_POPUP, {
        title: 'Specify variable name',
        url,
        width: 400,
        height: 500,
      });
    }
  }
  openVariablePane() {
    this.electronService.send(events.CREATE_MENU_POPUP, {
      title: 'Variables',
      url: "#/headless/variable-pane",
      width: 400,
      height: 450,
      alwaysOnTop: true,
      modal: false,
      raiseIfPresent: true,
    });
  }
}
