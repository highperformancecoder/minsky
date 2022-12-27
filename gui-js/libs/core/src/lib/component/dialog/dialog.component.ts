import {
  AfterViewInit,
  ChangeDetectorRef,
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
    private ngZone: NgZone,
    private cd: ChangeDetectorRef,
  ) {
    this.multipleKeyString = new FormControl('');
  }

  ngAfterViewInit(): void {
    this.inputHtmlElement = this.inputElement.nativeElement as HTMLElement;
    this.inputHtmlElement.focus();
  }

  setValue(value: string) {
    this.multipleKeyString.setValue(value);
    this.cd.detectChanges();
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
