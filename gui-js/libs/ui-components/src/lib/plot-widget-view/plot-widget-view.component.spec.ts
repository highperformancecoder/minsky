import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PlotWidgetViewComponent } from './plot-widget-view.component';

describe('PlotWidgetViewComponent', () => {
  let component: PlotWidgetViewComponent;
  let fixture: ComponentFixture<PlotWidgetViewComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [PlotWidgetViewComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PlotWidgetViewComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
