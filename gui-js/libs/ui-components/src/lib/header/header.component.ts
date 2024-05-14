import { ChangeDetectorRef, Component, OnDestroy, OnInit } from '@angular/core';
import { CommunicationService, ElectronService } from '@minsky/core';
import {
  events,
  HeaderEvent,
  RecordingStatus,
  ReplayRecordingStatus,
} from '@minsky/shared';
import { Subject, takeUntil } from 'rxjs';
import { NgClass } from '@angular/common';

@Component({
    selector: 'minsky-header',
    templateUrl: './header.component.html',
    styleUrls: ['./header.component.scss'],
    standalone: true,
    imports: [NgClass],
})
export class HeaderComponent implements OnInit, OnDestroy {
  headerEvent = 'HEADER_EVENT';
  isRecordingOn = false;
  isReplayRecordingOn = false;
  showPlayButton = false;

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

      this.commService.showPlayButton$.pipe(takeUntil(this.destroy$)).subscribe((showPlayButton) => {
        this.showPlayButton = showPlayButton;
        this.changeDetectorRef.detectChanges();
      });

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

  playButton() {
    if (this.showPlayButton) {
      this.commService.sendEvent(this.headerEvent, {
        action: 'CLICKED',
        target: 'PLAY',
      });
    } else {
      this.commService.sendEvent(this.headerEvent, {
        action: 'CLICKED',
        target: 'PAUSE',
      });
    }

    this.commService.showPlayButton$.next(
      !this.commService.showPlayButton$.value
    );
  }

  resetButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'RESET',
    });
  }

  stepButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'STEP',
    });
  }

  simulationSpeed(value) {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'SIMULATION_SPEED',
      value: value,
    });
  }

  zoomOutButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'ZOOM_OUT',
    });
  }

  zoomInButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'ZOOM_IN',
    });
  }

  resetZoomButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'RESET_ZOOM',
    });
  }

  zoomToFitButton() {
    this.commService.sendEvent(this.headerEvent, {
      action: 'CLICKED',
      target: 'ZOOM_TO_FIT',
    });
  }
  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
