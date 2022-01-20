import { ComponentFixture, TestBed } from '@angular/core/testing';
import { SimulationParametersComponent } from './simulation-parameters.component';

describe('SimulationParametersComponent', () => {
  let component: SimulationParametersComponent;
  let fixture: ComponentFixture<SimulationParametersComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [SimulationParametersComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(SimulationParametersComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
