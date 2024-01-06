import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { EditGroupComponent } from './edit-group.component';

describe('EditGroupComponent', () => {
  let component: EditGroupComponent;
  let fixture: ComponentFixture<EditGroupComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditGroupComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
