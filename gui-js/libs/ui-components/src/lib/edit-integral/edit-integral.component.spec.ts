import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditIntegralComponent } from './edit-integral.component';

describe('EditIntegralComponent', () => {
  let component: EditIntegralComponent;
  let fixture: ComponentFixture<EditIntegralComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [EditIntegralComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditIntegralComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
