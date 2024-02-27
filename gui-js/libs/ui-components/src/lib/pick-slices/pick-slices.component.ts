import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import { MessageBoxSyncOptions } from 'electron/renderer';
import { MatButtonModule } from '@angular/material/button';
import { NgFor } from '@angular/common';

@Component({
    selector: 'minsky-pick-slices',
    templateUrl: './pick-slices.component.html',
    styleUrls: ['../generic-form.scss', './pick-slices.component.scss'],
    standalone: true,
    imports: [NgFor, MatButtonModule],
})
export class PickSlicesComponent implements OnInit {
  sliceLabels: {label: string, selected: boolean, lastClicked: boolean}[] = [];

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
        selected: pickedSliceLabels.includes(l),
        lastClicked: false
      }));
    });
  }

  async handleSave() {
    if (this.electronService.isElectron) {
      if(this.sliceLabels.filter(sl => sl.selected).length === 0) {
        const options: MessageBoxSyncOptions = {
          buttons: ['OK'],
          message: `At least one slice label has to be selected.`,
          title: 'No slice labels selected',
        };

        await this.electronService.showMessageBoxSync(options);
      } else {
        await this.electronService.savePickSlices({
          command: this.command,
          handleIndex: this.handleIndex,
          pickedSliceLabels: this.sliceLabels.filter(sl => sl.selected).map(sl => sl.label)
        });
      }
    }
    this.closeWindow();
  }

  toggleSelected($event, sliceLabel) {
    if($event.ctrlKey) {
      sliceLabel.selected = !sliceLabel.selected;
    } else if($event.shiftKey) {
      const newValue = !sliceLabel.selected;
      const lastLabelIndex = this.sliceLabels.findIndex(sl => sl.lastClicked);
      if(lastLabelIndex === -1) {
        sliceLabel.selected = newValue;
      } else {
        const clickedIndex = this.sliceLabels.findIndex(sl => sl === sliceLabel);
        for(let i = Math.min(lastLabelIndex, clickedIndex); i < Math.max(lastLabelIndex, clickedIndex); i++) {
          this.sliceLabels[i].selected = newValue;
        }
      }
    } else {
      for(const sl of this.sliceLabels) {
        sl.selected = sl === sliceLabel;
      }
    }

    for(const sl of this.sliceLabels) {
      sl.lastClicked = sl === sliceLabel;
    }
  }

  selectAll(selected: boolean) {
    for(const sl of this.sliceLabels) sl.selected = selected;
  }

  closeWindow() {this.electronService.closeWindow();}
}
