import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PlotWidgetViewComponent } from './plot-widget-view.component';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { RouterTestingModule } from '@angular/router/testing';

describe('PlotWidgetViewComponent', () => {
  let component: PlotWidgetViewComponent;
  let fixture: ComponentFixture<PlotWidgetViewComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      providers: [ElectronService, WindowUtilityService],
      imports: [PlotWidgetViewComponent, RouterTestingModule],
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
