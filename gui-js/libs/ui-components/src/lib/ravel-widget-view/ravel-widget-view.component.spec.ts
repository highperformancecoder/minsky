import { ComponentFixture, TestBed } from '@angular/core/testing';

import { RavelViewComponent } from './ravel-widget-view.component';
import { RouterTestingModule } from '@angular/router/testing';

describe('RavelViewComponent', () => {
  let component: RavelViewComponent;
  let fixture: ComponentFixture<RavelViewComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [RavelViewComponent, RouterTestingModule],
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
