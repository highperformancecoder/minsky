import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';

@Component({
  selector: 'minsky-pick-slices',
  templateUrl: './pick-slices.component.html',
  styleUrls: ['../generic-form.scss', './pick-slices.component.scss'],
})
export class PickSlicesComponent implements OnInit {
  sliceLabels: {label: string, selected: boolean}[] = [];

  command: string;
  handleIndex: number;
  
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.command=params['command'];
      this.handleIndex = +params['handleIndex'];

      const pickedSliceLabels = params['pickedSliceLabels'].split(',');
      this.sliceLabels = params['allSliceLabels'].split(',').map(l => ({
        label: l,
        selected: pickedSliceLabels.includes(l)
      }));
    });
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      await this.electronService.savePickSlices({
        command: this.command,
        handleIndex: this.handleIndex,
        pickedSliceLabels: this.sliceLabels.filter(sl => sl.selected).map(sl => sl.label)
      });
    }
    this.closeWindow();
  }

  selectAll(selected: boolean) {
    for(const sl of this.sliceLabels) sl.selected = selected;
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
