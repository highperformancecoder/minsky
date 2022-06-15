import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditUserFunctionComponent } from './edit-user-function.component';

describe('EditUserFunctionComponent', () => {
  let component: EditUserFunctionComponent;
  let fixture: ComponentFixture<EditUserFunctionComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [EditUserFunctionComponent],
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
