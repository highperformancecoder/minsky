import { MatDialogRef, MAT_DIALOG_DATA, MatDialog } from '@angular/material/dialog';
import { Inject, Component, ViewEncapsulation } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { MatFormField } from '@angular/material/form-field';
import { LatexDirective } from '../../directives/latex.directive';

export class EditVariableDialogData {
  name: string;
  symbol: string;
  description: string;
  type: string;
  value?: number;
  minimum?: number;
  maximum?: number;
}

@Component({
    selector: 'edit-variable-dialog',
    templateUrl: './editvariabledialog.component.html',
    styleUrls: ['./editvariabledialog.component.css'],
    encapsulation: ViewEncapsulation.None,
    standalone: true,
  imports: [FormsModule, MatFormField, LatexDirective]
  })
  export class EditVariableDialog {
    result: EditVariableDialogData;

    get valueTag() {
      return this.result.type === 'constant' ? 'Value' : 'Initial value';
    }

    constructor(public dialog: MatDialog,
      public dialogRef: MatDialogRef<EditVariableDialog>,
      @Inject(MAT_DIALOG_DATA) public data: EditVariableDialogData) {
        this.result = data;
      }
  
    onCancelClick(): void {
      this.dialogRef.close();
    }

    onOKClick(): void {
      this.dialogRef.close(this.result);
    }
  }
