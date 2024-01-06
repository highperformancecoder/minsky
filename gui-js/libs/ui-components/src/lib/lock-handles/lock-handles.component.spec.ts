import { ComponentFixture, TestBed } from '@angular/core/testing';

import { LockHandlesComponent } from './lock-handles.component';
import { RouterTestingModule } from '@angular/router/testing';

describe('LockHandlesComponent', () => {
  let component: LockHandlesComponent;
  let fixture: ComponentFixture<LockHandlesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [LockHandlesComponent, RouterTestingModule],
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
