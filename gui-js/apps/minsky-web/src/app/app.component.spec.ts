import { TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { TranslateModule } from '@ngx-translate/core';
import { AppComponent } from './app.component';
import { MatDialog, MatDialogRef } from '@angular/material/dialog'

describe('AppComponent', () => {
  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        declarations: [AppComponent],
        providers: [{ provide: MatDialog, useValue: {} }, { provide: MatDialogRef, useValue: {} }],
        imports: [RouterTestingModule, TranslateModule.forRoot()],
      }).compileComponents();
    })
  );

  it(
    'should create the app',
    waitForAsync(() => {
      const fixture = TestBed.createComponent(AppComponent);
      const app = fixture.debugElement.componentInstance;
      expect(app).toBeTruthy();
    })
  );
});
