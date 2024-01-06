import {
  ChangeDetectorRef,
  Component,
  EventEmitter,
  OnDestroy,
  OnInit,
  Output,
} from '@angular/core';
import { CommunicationService } from '@minsky/core';
import { HeaderEvent } from '@minsky/shared';
import { Subject, takeUntil } from 'rxjs';

@Component({
    selector: 'minsky-toolbar',
    templateUrl: './toolbar.component.html',
    styleUrls: ['./toolbar.component.scss'],
    standalone: true,
})
export class ToolbarComponent implements OnInit, OnDestroy {
  private static availableOperationsMap = null;

  destroy$ = new Subject<{}>();

  @Output() toolbarEvent = new EventEmitter<HeaderEvent>();

  headerEvent = 'HEADER_EVENT';

  constructor(
    public communicationService: CommunicationService,
    private changeDetectorRef: ChangeDetectorRef
  ) {}

  showPlayButton = false;

  ngOnInit(): void {
    this.communicationService.showPlayButton$.pipe(takeUntil(this.destroy$)).subscribe((showPlayButton) => {
      this.showPlayButton = showPlayButton;
      this.changeDetectorRef.detectChanges();
    });
  }

  playButton() {
    if (this.showPlayButton) {
      this.toolbarEvent.emit({
        action: 'CLICKED',
        target: 'PLAY',
      });
    } else {
      this.toolbarEvent.emit({
        action: 'CLICKED',
        target: 'PAUSE',
      });
    }

    this.communicationService.showPlayButton$.next(
      !this.communicationService.showPlayButton$.value
    );
  }

  resetButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'RESET',
    });
  }

  stepButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'STEP',
    });
  }

  simulationSpeed(value) {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'SIMULATION_SPEED',
      value: value,
    });
  }

  zoomOutButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'ZOOM_OUT',
    });
  }

  zoomInButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'ZOOM_IN',
    });
  }

  resetZoomButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'RESET_ZOOM',
    });
  }

  zoomToFitButton() {
    this.toolbarEvent.emit({
      action: 'CLICKED',
      target: 'ZOOM_TO_FIT',
    });
  }

  // closeTextInput() {
  //   TextInputUtilities.hide();
  // }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
