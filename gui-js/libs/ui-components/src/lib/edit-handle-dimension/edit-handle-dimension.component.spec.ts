import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { EditHandleDimensionComponent } from './edit-handle-dimension.component';

describe('EditHandleDimensionComponent', () => {
  let component: EditHandleDimensionComponent;
  let fixture: ComponentFixture<EditHandleDimensionComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditHandleDimensionComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditHandleDimensionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
