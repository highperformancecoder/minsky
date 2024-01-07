import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditUserFunctionComponent } from './edit-user-function.component';
import { ElectronService } from '@minsky/core';
import { RouterTestingModule } from '@angular/router/testing';

describe('EditUserFunctionComponent', () => {
  let component: EditUserFunctionComponent;
  let fixture: ComponentFixture<EditUserFunctionComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditUserFunctionComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditUserFunctionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
