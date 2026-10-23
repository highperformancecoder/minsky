import { ComponentFixture, TestBed } from '@angular/core/testing';

import { LogSimulationComponent } from './log-simulation.component';

describe('LogSimulationComponent', () => {
  let component: LogSimulationComponent;
  let fixture: ComponentFixture<LogSimulationComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [LogSimulationComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(LogSimulationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
