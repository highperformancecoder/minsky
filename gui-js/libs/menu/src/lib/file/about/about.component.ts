import { Component, OnInit, VERSION, ChangeDetectorRef } from '@angular/core';
import { ElectronService } from '@minsky/core';
import { version } from '@minsky/shared';

@Component({
    selector: 'minsky-about',
    templateUrl: './about.component.html',
    styleUrls: ['./about.component.scss'],
    standalone: true,
})
export class AboutComponent implements OnInit {
  angularVersion = VERSION.full;
  minskyVersion: string;
  frontEndVersion: string;
  ravelVersion: string;

  constructor(private electronService: ElectronService, private cdRef: ChangeDetectorRef,){}

  ngOnInit(): void {
    (async () => {
      let minsky=this.electronService.minsky;
      this.ravelVersion = await minsky.ravelVersion();
      this.minskyVersion = await minsky.minskyVersion();
      this.frontEndVersion=version;
      // Ensure UI updates even if backend calls resolve outside Angular's zone.
      this.cdRef.detectChanges();    })();
  }

  async closeWindow() {this.electronService.closeWindow();}
}
