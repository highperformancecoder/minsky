import { ComponentFixture, TestBed } from '@angular/core/testing';
import { AvailableOperationsComponent } from './available-operations.component';

describe('AvailableOperationsComponent', () => {
  let component: AvailableOperationsComponent;
  let fixture: ComponentFixture<AvailableOperationsComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AvailableOperationsComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(AvailableOperationsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
