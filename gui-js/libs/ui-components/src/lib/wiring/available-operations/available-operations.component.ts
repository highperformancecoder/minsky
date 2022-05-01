import { Component, Input } from '@angular/core';
import { CommunicationService } from '@minsky/core';

@Component({
  selector: 'minsky-available-operations',
  templateUrl: './available-operations.component.html',
  styleUrls: ['./available-operations.component.scss'],
})
export class AvailableOperationsComponent {
  @Input() title = '';
  @Input() operations: string[] = [];

  constructor(public communicationService: CommunicationService) {}
}
