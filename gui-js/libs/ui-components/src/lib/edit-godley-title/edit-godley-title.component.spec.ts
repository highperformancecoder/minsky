import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { EditGodleyTitleComponent } from './edit-godley-title.component';

describe('EditGodleyTitleComponent', () => {
  let component: EditGodleyTitleComponent;
  let fixture: ComponentFixture<EditGodleyTitleComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditGodleyTitleComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditGodleyTitleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
