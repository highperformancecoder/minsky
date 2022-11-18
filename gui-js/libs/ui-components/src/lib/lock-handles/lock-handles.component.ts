import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService } from '@minsky/core';
import {Ravel} from '@minsky/shared';
import * as JSON5 from 'json5';

@Component({
  selector: 'minsky-lock-handles',
  templateUrl: './lock-handles.component.html',
  styleUrls: ['../generic-form.scss', './lock-handles.component.scss'],
})
export class LockHandlesComponent implements OnInit {
  handleLockInfo: any[];
  ravelNames: string[];
  lockHandles: string[];

  attributes = [{
    label: 'Slicer',
    key: 'slicer',
    tooltip: 'Slicer includes whether a handle is an output handle or not'
  }, {
    label: 'Orientation',
    key: 'orientation',
    tooltip: 'Orientation includes whether a handle is collapsed or not'
  },
  {
    label: 'Calipers',
    key: 'calipers',
    tooltip: ''
  },
  {
    label: 'Sort order',
    key: 'order',
    tooltip: 'Sort order includes picked slices'
  }
];

  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService
  ) {}

  ngOnInit(): void {
    this.route.queryParams.subscribe((params) => {
      this.handleLockInfo = JSON5.parse(params['handleLockInfo']);
      this.ravelNames = params['ravelNames'].split(',');
      this.lockHandles = params['lockHandles'].split(',');
    });
  }

  handleSave() {
    if (this.electronService.isElectron) {
      let ravel=new Ravel(this.electronService.minsky.canvas.item);
      ravel.lockGroup.handleLockInfo.properties(this.handleLockInfo);
      ravel.lockGroup.validateLockHandleInfo();
    }
    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
