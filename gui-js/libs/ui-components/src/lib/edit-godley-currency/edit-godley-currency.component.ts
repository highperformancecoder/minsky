import { Component } from '@angular/core';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { GodleyIcon } from '@minsky/shared';
import { InputModalComponent } from '../input-modal/input-modal.component';

@Component({
    selector: 'minsky-edit-godley-currency',
    templateUrl: './edit-godley-currency.component.html',
    styleUrls: ['./edit-godley-currency.component.scss'],
    standalone: true,
    imports: [InputModalComponent],
})
export class EditGodleyCurrencyComponent {
  currency = '';

  constructor(
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService
  ) {}

  async handleSaveCurrency(newCurrency: string) {
    if (this.electronService.isElectron) {
      new GodleyIcon(this.electronService.minsky.canvas.item).setCurrency(newCurrency);
      this.windowUtilityService.closeCurrentWindowIfNotMain();
    }
  }
}
