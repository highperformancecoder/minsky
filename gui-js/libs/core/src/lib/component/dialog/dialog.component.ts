import {
  AfterViewInit,
  Component,
  ElementRef,
  NgZone,
  ViewChild,
} from '@angular/core';
import { FormControl } from '@angular/forms';
import { MatDialogRef } from '@angular/material/dialog';

@Component({
  selector: 'minsky-dialog',
  templateUrl: './dialog.component.html',
  styleUrls: ['./dialog.component.scss'],
})
export class DialogComponent implements AfterViewInit {
  @ViewChild('inputElement') inputElement: ElementRef;
  inputHtmlElement: HTMLElement;
  multipleKeyString: FormControl;

  constructor(
    public dialogRef: MatDialogRef<DialogComponent>,
    private ngZone: NgZone
  ) {
    this.multipleKeyString = new FormControl('');
  }

  ngAfterViewInit(): void {
    this.inputHtmlElement = this.inputElement.nativeElement as HTMLElement;
    this.inputHtmlElement.focus();
  }

  submit(): void {
    this.ngZone.run(() => {
      this.dialogRef.close(this.multipleKeyString.value);
    });
  }

  close(): void {
    this.ngZone.run(() => {
      this.dialogRef.close();
    });
  }
}
