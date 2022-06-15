import { Component } from '@angular/core';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';

@Component({
  selector: 'minsky-edit-godley-currency',
  templateUrl: './edit-godley-currency.component.html',
  styleUrls: ['./edit-godley-currency.component.scss'],
})
export class EditGodleyCurrencyComponent {
  currency = '';

  constructor(
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService
  ) {}

  async handleSaveCurrency(newCurrency: string) {
    if (this.electronService.isElectron) {
      await this.electronService.sendMinskyCommandAndRender({
        command: `${commandsMapping.CANVAS_ITEM_SET_CURRENCY} "${newCurrency}"`,
      });
      this.windowUtilityService.closeCurrentWindowIfNotMain();
    }
  }
}
