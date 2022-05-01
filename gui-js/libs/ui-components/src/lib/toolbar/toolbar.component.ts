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
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-toolbar',
  templateUrl: './toolbar.component.html',
  styleUrls: ['./toolbar.component.scss'],
})
export class ToolbarComponent implements OnInit, OnDestroy {
  private static availableOperationsMap = null;

  @Output() toolbarEvent = new EventEmitter<HeaderEvent>();

  headerEvent = 'HEADER_EVENT';

  constructor(
    public communicationService: CommunicationService,
    private changeDetectorRef: ChangeDetectorRef
  ) {}

  showPlayButton = false;

  ngOnInit(): void {
    this.communicationService.showPlayButton$.subscribe((showPlayButton) => {
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

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
