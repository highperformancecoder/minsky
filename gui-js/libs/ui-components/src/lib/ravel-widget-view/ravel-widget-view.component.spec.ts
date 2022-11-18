import { ComponentFixture, TestBed } from '@angular/core/testing';

import { RavelViewComponent } from './ravel-widget-view.component';

describe('RavelViewComponent', () => {
  let component: RavelViewComponent;
  let fixture: ComponentFixture<RavelViewComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [RavelViewComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(RavelViewComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
