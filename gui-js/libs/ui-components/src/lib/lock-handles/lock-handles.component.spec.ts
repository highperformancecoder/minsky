import { ComponentFixture, TestBed } from '@angular/core/testing';

import { LockHandlesComponent } from './lock-handles.component';

describe('LockHandlesComponent', () => {
  let component: LockHandlesComponent;
  let fixture: ComponentFixture<LockHandlesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [LockHandlesComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(LockHandlesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
