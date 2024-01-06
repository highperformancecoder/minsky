import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { VariablePaneComponent } from './variable-pane.component';

describe('VariablePaneComponent', () => {
  let component: VariablePaneComponent;
  let fixture: ComponentFixture<VariablePaneComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [VariablePaneComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(VariablePaneComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
