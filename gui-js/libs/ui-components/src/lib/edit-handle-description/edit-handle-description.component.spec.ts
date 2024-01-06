import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { EditHandleDescriptionComponent } from './edit-handle-description.component';

describe('EditHandleDescriptionComponent', () => {
  let component: EditHandleDescriptionComponent;
  let fixture: ComponentFixture<EditHandleDescriptionComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditHandleDescriptionComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditHandleDescriptionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
