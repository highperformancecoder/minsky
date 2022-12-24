import { Component, Input, AfterViewInit } from '@angular/core';
import { CommunicationService } from '@minsky/core';

@Component({
  selector: 'minsky-available-operations',
  templateUrl: './available-operations.component.html',
  styleUrls: ['./available-operations.component.scss'],
})
export class AvailableOperationsComponent {
  @Input() title = '';
  @Input() operations: string[] = [];

  constructor(public communicationService: CommunicationService) {
  }

  onButtonDrag(event, operation) {
    const be = document.querySelector<HTMLElement>(`#button${operation}`);
    be.click();
    event.event.stopPropagation();
    event.event.preventDefault();
  }
}
