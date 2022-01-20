import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BackgroundColorComponent } from './background-color.component';

describe('BackgroundColorComponent', () => {
  let component: BackgroundColorComponent;
  let fixture: ComponentFixture<BackgroundColorComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [BackgroundColorComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(BackgroundColorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
