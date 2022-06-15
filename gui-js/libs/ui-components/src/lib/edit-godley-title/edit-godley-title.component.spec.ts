import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditGodleyTitleComponent } from './edit-godley-title.component';

describe('EditGodleyTitleComponent', () => {
  let component: EditGodleyTitleComponent;
  let fixture: ComponentFixture<EditGodleyTitleComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [EditGodleyTitleComponent],
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
