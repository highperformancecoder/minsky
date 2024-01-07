import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PlotWidgetOptionsComponent } from './plot-widget-options.component';
import { RouterTestingModule } from '@angular/router/testing';

describe('PlotWidgetOptionsComponent', () => {
  let component: PlotWidgetOptionsComponent;
  let fixture: ComponentFixture<PlotWidgetOptionsComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [PlotWidgetOptionsComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PlotWidgetOptionsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
