import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditDescriptionComponent } from './edit-description.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ElectronService } from '@minsky/core';

describe('EditDescriptionComponent', () => {
  let component: EditDescriptionComponent;
  let fixture: ComponentFixture<EditDescriptionComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      providers: [ElectronService],
      imports: [EditDescriptionComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditDescriptionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
