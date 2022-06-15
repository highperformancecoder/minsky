import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GodleyWidgetViewComponent } from './godley-widget-view.component';

describe('GodleyWidgetViewComponent', () => {
  let component: GodleyWidgetViewComponent;
  let fixture: ComponentFixture<GodleyWidgetViewComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [GodleyWidgetViewComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(GodleyWidgetViewComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
