import { ChangeDetectorRef, Component, OnDestroy, OnInit } from '@angular/core';
import { CommunicationService, ElectronService } from '@minsky/core';
import {
  events,
  HeaderEvent,
  RecordingStatus,
  ReplayRecordingStatus,
} from '@minsky/shared';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'minsky-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.scss'],
})
export class HeaderComponent implements OnInit, OnDestroy {
  headerEvent = 'HEADER_EVENT';
  isRecordingOn = false;
  isReplayRecordingOn = false;

  destroy$ = new Subject<{}>();

  constructor(
    public commService: CommunicationService,
    private electronService: ElectronService,
    private changeDetectorRef: ChangeDetectorRef
  ) {}
  ngOnInit() {
    if (this.electronService.isElectron) {
      this.electronService.on(
        events.RECORDING_STATUS_CHANGED,
        (event, { status }) => {
          switch (status) {
            case RecordingStatus.RecordingStarted:
              this.isRecordingOn = true;
              this.changeDetectorRef.detectChanges();
              break;

            case RecordingStatus.RecordingStopped:
            case RecordingStatus.RecordingCanceled:
              this.isRecordingOn = false;
              this.changeDetectorRef.detectChanges();
              break;

            default:
              break;
          }
        }
      );

      this.commService.ReplayRecordingStatus$.pipe(takeUntil(this.destroy$)).subscribe(
        (status: ReplayRecordingStatus) => {
          switch (status) {
            case ReplayRecordingStatus.ReplayStarted:
              this.isReplayRecordingOn = true;
              this.changeDetectorRef.detectChanges();
              break;

            case ReplayRecordingStatus.ReplayStopped:
              this.isReplayRecordingOn = false;
              this.changeDetectorRef.detectChanges();
              break;

            default:
              break;
          }
        }
      );
    }
  }

  async handleToolbarEvent(event: HeaderEvent) {
    await this.commService.sendEvent(this.headerEvent, event);
  }

  recalculateButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'RESET',
    });
  }

  recordButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'RECORD',
    });
  }

  recordingReplyButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'RECORDING_REPLAY',
    });
  }

  reverseCheckboxButton(event) {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'REVERSE_CHECKBOX',
      value: event.target.checked,
    });
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
